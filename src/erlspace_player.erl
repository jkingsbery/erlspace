-module(erlspace_player).

-export([new/1,up/1,down/1,left/1,right/1,to_json/1]).

-record(player_data,{name,x=0,y=0}).

new(Name)->
    #player_data{name=Name}.

up(Record)->
    Record#player_data{y=Record#player_data.y+1}.
    
down(Record)->
    Record#player_data{y=Record#player_data.y-1}.
    
right(Record)->
    Record#player_data{x=Record#player_data.x+1}.
    
left(Record)->
    Record#player_data{x=Record#player_data.x-1}.
    
to_json(Record)->
    jiffy:encode({[{name,list_to_binary(Record#player_data.name)},
		   {x,Record#player_data.x},
		   {y,Record#player_data.y}]}).
