
-module(erlspace_sup).

-behaviour(supervisor).

%% API
-export([start_link/0]).

%% Supervisor callbacks
-export([init/1]).

%% Helper macro for declaring children of supervisor
-define(CHILD(I, Type), {I, {I, start_link, []}, permanent, 5000, Type, [I]}).

-define(NETWORKING,{erlspace_networking,
		    {erlspace_networking,start_link,[1234]},
		    permanent,brutal_kill,
		    worker,
		    [erlspace_networking]}).
-define(SECTOR_A,{sectorA,
		    {erlspace_sector,start_link,[]},
		    permanent,brutal_kill,
		    worker,
		    [erlspace_sector]}).

%% ===================================================================
%% API functions
%% ===================================================================

start_link() ->
    supervisor:start_link({local, ?MODULE}, ?MODULE, []).

%% ===================================================================
%% Supervisor callbacks
%% ===================================================================

init([]) ->
    {ok, { {one_for_one, 5, 10}, [?NETWORKING,?SECTOR_A]} }.

stop()->
    exit(whereis(?MODULE),kill).

