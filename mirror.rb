#!/usr/bin/env ruby
#
# $Id$
#
# Copyright (C) 2002-2003 University of Southampton
# Copyright (C) 2003 Simon Howard
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

#
# Mirroring server
#

require 'irmo'

class MirrorServer
	def initialize(type, host, port, remote_ifspec, local_ifspec)
		@local_world = Irmo::World.new(local_ifspec)
		@conn = Irmo::connect(type, host, port, remote_ifspec,
				      @local_world)

		puts "connected to #{host}"

		@local_ifspec = local_ifspec
		@remote_ifspec = remote_ifspec

		@socket = Irmo::Socket.new(type, port)
		@server = @socket.new_server(nil, @conn.world, local_ifspec)

		@s2cobj = {}
		@c2sobj = {}

		@server.watch_connect do |cl|
			new_client(cl)
		end
	end

	def disconnect_client(cl)

		# delete all the copied objects

		puts "disconnect"

		@c2sobj[cl].each_value do |obj|
			@s2cobj.delete(obj.id)
			puts "delete #{obj.get_class},#{obj.id}"
			obj.destroy
		end

		@c2sobj.delete(cl)
	end

	def new_client(cl)
		
		c2sobj = {}
		@c2sobj[cl] = c2sobj

		cl.world.watch_new do |obj|

			# make a copy in the local world

			obj2 = @local_world.new_object(obj.get_class)

			c2sobj[obj.id] = obj2
			@s2cobj[obj2.id] = { 'obj' => obj, 'client' => cl }
		end

		cl.world.watch do |obj,var|
			obj2 = c2sobj[obj.id]

			puts "set #{obj.id}::#{var}"
			# copy it

			obj2.set(var, obj.get(var))
		end

		cl.world.watch_destroy do |obj|

			# remove the copy of the object and destroy it

			obj2 = c2sobj[obj.id]
			c2sobj.delete(obj.id)
			s2cobj.delete(obj2.id)
			obj2.destroy
		end

		cl.watch_disconnect do
			disconnect_client(cl)
		end
	end

	def shutdown
		@conn.disconnect
	end

	def run
		@conn.run
		@socket.run
	end
end

class IrmoroidsMirrorServer < MirrorServer
	def initialize(type, host, port)
		local_ifspec = Irmo::InterfaceSpec.new('interfaces/astroclient.if')
		remote_ifspec = Irmo::InterfaceSpec.new('interfaces/astro.if')

		super(type, host, port, remote_ifspec, local_ifspec)

		@server.watch_connect do 
			puts "got a new connection"
		end

		@local_world.watch_method('assoc_player') do |clobj,svobj|

			# translate the local world clobj to the client
			# one

			translation = @s2cobj[clobj]

			world = translation['client'].world

			world.assoc_player(translation['obj'].id, svobj)
					
		end
	end
end

server = IrmoroidsMirrorServer.new('IPV4', 'random', 2048)

puts "server started"

begin
	loop do
		server.run
	end
ensure
	puts "shutting down"
	server.shutdown
end

# $Log$
# Revision 1.1  2003/09/01 17:07:39  fraggle
# Irmoroids mirroring server
#

