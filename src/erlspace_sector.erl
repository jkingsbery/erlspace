-module(erlspace_sector).

-behavior(gen_server).

-record(player_data,{name,x=0,y=0}).

-export([start_link/0]).
-export([init/1,handle_call/3]).
-export([add_client/1,remove_client/1,broadcast/1,status/0]).

start_link()->
    gen_server:start_link({local,sectorA},erlspace_sector,[],[]).

init(_Args)->
    {ok,[]}.


add_client(Socket)->
    gen_server:call(sectorA,{add_client,Socket}).

remove_client(Socket)->
    gen_server:call(sectorA,{remove_client,Socket}).

broadcast(Message)->
    gen_server:call(sectorA,{broadcast,Message}).

status()->
    gen_server:call(sectorA,status).

handle_call(status,_From,State)->
    {reply, State, State};
handle_call({add_client,Socket},_From,State)->
    {reply, ok, [Socket | State]};
handle_call({remove_client,Socket},_From,State)->
    NewList=lists:delete(Socket,State),
    {reply, ok,NewList};
handle_call({broadcast,Message},_From,State)->
    broadcast_helper(State,Message),
    {reply, ok, State}.

broadcast_helper([],_Message)->
    ok;
broadcast_helper([X|List],Message)->
    gen_tcp:send(X,to_string(Message)),
    broadcast_helper(List,Message).

to_string(P=#player_data{})->
    erlspace_player:to_json(P);
to_string(Object)->
    io_lib:format("~p~n",[Object]).
