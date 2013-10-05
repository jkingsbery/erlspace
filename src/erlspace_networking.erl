-module(erlspace_networking).

% This module handles the TCP connections. There are 2+n processes: 1 for accepting
% TCP connections, 1 letting us stop, and 1 each for the n connected clients.

% TODO: Split this into the part that accepts connections and the part that handles the client I/O

% TCP Exports
-export([server/1,do_recv/2,init_connection/1]).
% gen_server exports
-export([init/1,handle_call/3,terminate/2]).
-export([start_link/1,stop/0]).
-export([process_command/2]).

-behavior(gen_server).

start_link(PortNumber) ->
    gen_server:start_link({local,networking},erlspace_networking,[PortNumber],[]).

init(PortNumber) ->
    case gen_tcp:listen(1234,[list,{active,false},{reuseaddr,true}]) of
	{ok, ListenSocket} ->
	    spawn(?MODULE,server,[ListenSocket]),
	    {ok, ListenSocket};
	{error, ErrorMsg} ->
		io:format("Error: ~s",[ErrorMsg]),
		{stop,ErrorMsg}
    end.

handle_call(stop,_From,ListenSocket)->
    gen_tcp:close(ListenSocket),
    {stop,normal,ListenSocket}.

stop() ->
    gen_server:call(networking,stop).

server(ListenSocket)->    
    accept_loop(ListenSocket),
    ok.

accept_loop(ListenSocket)->
    {ok, Socket} = gen_tcp:accept(ListenSocket),
    spawn(?MODULE,init_connection,[Socket]),
    accept_loop(ListenSocket).

% TODO: There should probably be some kind of FSM here to manage this.
init_connection(Socket)->
    case gen_tcp:recv(Socket,0) of
	{ok,"Name: "++Name0} ->
	    Name=string:left(Name0,string:len(Name0)-1),
	    io:format("~s joined!~n",[Name]),
	    gen_tcp:send(Socket,"ok\n"),
	    Player=erlspace_player:new(Name),
	    erlspace_sector:add_client(Socket),
	    do_recv(Socket,Player);
	{error,closed} ->
	    gen_tcp:close(Socket);
	{ok,B} ->
	    io:format("received ~p~n",[B]),
	    gen_tcp:send(Socket,"Please provide a name for the player.\n"),
	    gen_tcp:close(Socket)
    end.

process_command("up",Player)->
    erlspace_player:up(Player);
process_command("down",Player) ->
    erlspace_player:up(Player);
process_command("left",Player)->
    erlspace_player:left(Player);
process_command("right",Player) ->
    erlspace_player:right(Player).

do_recv(Sock,Player) ->
    case gen_tcp:recv(Sock,0) of
	{ok,B} ->
	    Command=string:left(B,string:len(B)-1),
	    io:format("received ~p~n",[Command]),
	    NewPlayer=process_command(Command,Player),
	    io:format("~p~n",[NewPlayer]),
	    erlspace_sector:broadcast(NewPlayer),
	    do_recv(Sock,NewPlayer);
	{error, closed} ->
	    erlspace_sector:remove_client(Sock),
	    gen_tcp:close(Sock)	
    end.

terminate(Reason,_NewState)->
    io:format("Terminating: ~s",[Reason]),
    ok.
