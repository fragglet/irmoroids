#!/usr/bin/env ruby
#
# Copyright (C) 2002-3 Simon Howard
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

	@@Translation = Struct.new('Translation', :client, :obj)

	def initialize(type, host, port, remote_ifspec, local_ifspec)
		@local_world = Irmo::World.new(local_ifspec)
		@conn = Irmo::connect(type, host, port, remote_ifspec,
				      @local_world)

		#puts "connected to #{host}"

		@local_ifspec = local_ifspec
		@remote_ifspec = remote_ifspec

		@socket = Irmo::Socket.new(type, port)
		@server = @socket.new_server(nil, @conn.world, local_ifspec)

		# hash table for each object in @local_world => client objects
		#
		# s2cobj[obj].client => client which created this object
		# s2cobj[obj].obj    => original client object

		@s2cobj = {}

		# hash table for translating client objects to server objects
		# c2sobj[client][obj.id] => serverobj

		@c2sobj = {}

		@server.watch_connect do |cl|
			new_client(cl)
		end
	end

	def disconnect_client(cl)

		# delete all the copied objects

		@c2sobj[cl].each_value do |obj|
			@s2cobj.delete(obj.id)
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
			@s2cobj[obj2.id] = @@Translation.new(cl, obj)
		end

		cl.world.watch do |obj,var|
			obj2 = c2sobj[obj.id]

			#puts "set #{obj.id}::#{var}"
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
		@server.shutdown
	end

	def block
		Irmo::Socket.block([@conn.socket, @socket], 50)
	end

	def run
		@conn.run
		@socket.run
	end
end

class IrmoroidsMirrorServer < MirrorServer
	IRMOROIDS_PORT = 2048

	def initialize(type, host)
		local_ifspec = Irmo::InterfaceSpec.new('interfaces/astroclient.if')
		remote_ifspec = Irmo::InterfaceSpec.new('interfaces/astro.if')

		super(type, host, IRMOROIDS_PORT, remote_ifspec, local_ifspec)

		@server.watch_connect do 
			#puts "got a new connection"
		end

		@local_world.watch_method('assoc_player') do |clobj,svobj|

			# translate the local world clobj to the client
			# one

			translation = @s2cobj[clobj]

			world = translation.client.world

			world.assoc_player(translation.obj.id, svobj)
					
		end
	end
end

if ARGV.length < 1 
	puts "Usage: #{$0} <hostname>"
	exit 0
end

server = IrmoroidsMirrorServer.new('IPV4', ARGV[0])

#puts "server started"

begin
	loop do
		server.block
		server.run
	end
ensure
	#puts "shutting down"
	server.shutdown
end

