rm -rf rel
mkdir rel
cd rel
rebar create-node nodeid=erlspace
cd ..
cp patch/rel/* rel/
rebar compile generate