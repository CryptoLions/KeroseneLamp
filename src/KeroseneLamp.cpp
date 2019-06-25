//###########################################################################################
//###########################################################################################
//##
//##  KeroseneLamp powerd by SimpleAssets (Digital Assets)
//##  (C) 2019 by CryptoLions [ https://CryptoLions.io ]
//##
//##  Medium Post: https://medium.com/@cryptolions/the-prospectors-lantern-anatomy-of-a-simple-asset-ae0b6cf4f9b
//##
//##
//##  SimpleAssets - simple standard for digital assets (ie. Non-Fungible Tokens) 
//##                 for EOSIO blockchains
//##
//##
//##    WebSite:        https://simpleassets.io
//##    GitHub:         https://github.com/CryptoLions/SimpleAssets.
//##    Presentation:   https://medium.com/@cryptolions/introducing-simple-assets-b4e17caafaa4
//##
//###########################################################################################
//###########################################################################################


#include <KeroseneLamp.hpp>
#include <json.hpp>
#include <eosiolib/transaction.hpp>

using json = nlohmann::json;


ACTION KeroseneLamp::getfuel( name acc ){

	require_auth( acc );

	conf config(_self, _self.value);
	_cstate = config.exists() ? config.get() : global{};

	check ( _cstate.owner == acc, "FUEL faucet will work for you when you will have Lamp in your hends...");

	accounts acnts( SIMPLEASSETSCONTRACT, acc.value );
	auto it = acnts.find( FUELTOKENID );

	uint64_t bal = 0;
	if (it != acnts.end()) {
		bal = it->balance.amount;	
	}

	check ( bal < 10000, "Your backet is for 100 FUEL Max");


	std::string memo = "FUEL faucet";
	action saRes1 = action(
		permission_level{get_self(), "active"_n},
		SIMPLEASSETSCONTRACT,
		"issuef"_n,
		std::make_tuple(acc, get_self(), asset(10000-bal, TFUEL), memo)
	);
	saRes1.send();

}


ACTION KeroseneLamp::saecreate( name owner, uint64_t assetid ) {
   checkTicker();
}


ACTION KeroseneLamp::saetransfer( name from, name to, std::vector<uint64_t>& assetids, string memo ) {
	conf config(_self, _self.value);
	_cstate = config.exists() ? config.get() : global{};
		
	for( size_t i = 0; i < assetids.size(); i++ ) {
		if (assetids[i] == LAMPID){
			_cstate.owner = to;
			_cstate.passtime = now();
			config.set(_cstate, _self);		
		}						
	}
   
	useFuel(0);
	checkTicker();
}


ACTION KeroseneLamp::saeclaim( name who, std::map< uint64_t, name >& assetids ) {
	conf config(_self, _self.value);
	_cstate = config.exists() ? config.get() : global{};

	auto assetidsIt = assetids.begin(); 
	while(assetidsIt != assetids.end() ) {
		uint64_t keyid = (*assetidsIt).first; 

		if (keyid == LAMPID){
			//Check if alread passed a lamp, if not - send bonus FT and record to table
			passes passmade(_self, _self.value);
			auto itr = passmade.find( assetids[keyid].value );				
			if (itr == passmade.end()) {
				passmade.emplace( _self, [&]( auto& s ) {     
					s.from = assetids[keyid];
					s.to = who;
					s.pdate = now();
				});	


				//Check Bonus FT (tickets) balance 
				accounts acnts( SIMPLEASSETSCONTRACT, get_self().value );
				auto it = acnts.find( BONUS_FTID );

				if (it != acnts.end()){
					if (it->balance.amount > 0) {
						string memo_tb = "Gold Ticket (prospectors.io)";
						action sendAsset = action(
							permission_level{get_self(),"active"_n},
							SIMPLEASSETSCONTRACT,
							"transferf"_n,
							std::make_tuple(get_self(), assetids[keyid], BONUSFGCONTRACT, BONUS_QUANTITY, memo_tb)
						);
						sendAsset.send(); // Transfer Bonus
					}
				}
			}
			_cstate.owner = who;
			_cstate.passtime = now();
			config.set(_cstate, _self);		
		}						
		assetidsIt++;
	}
	  
	useFuel(0);
	checkTicker();
}


//ACTION KeroseneLamp::saeburn( name who, std::vector<uint64_t>& assetids, string memo  ) {
//}


ACTION KeroseneLamp::ticker() {
	//only this contract can call this action
	require_auth(get_self());
	tick();
}


//------------------------------------------
//------ PRIVATE ---------------------------
void KeroseneLamp::checkTicker(){

	conf config(_self, _self.value);
	_cstate = config.exists() ? config.get() : global{};

	int timedftx = now() - _cstate.lastticker - tickertime;

	if (timedftx >= 0){
		tick();
	}

}


void KeroseneLamp::tick() {
	conf config(_self, _self.value);
	_cstate = config.exists() ? config.get() : global{};
	
	//eat fuel, if no fuel - switch lamp off and stop ticker
	bool alive = useFuel(0);
	if (alive){
		deftx(0);
		_cstate.lastticker = now();
		config.set(_cstate, _self);
	}

}

bool KeroseneLamp::useFuel(bool ison ){

	conf config(_self, _self.value);
	_cstate = config.exists() ? config.get() : global{};
	
	if (_cstate.owner == ""_n ) return false;
	
	sassets assets(SIMPLEASSETSCONTRACT, _cstate.owner.value);
	auto idx = assets.find(LAMPID);
	if (idx == assets.end()) return false;
	auto mdata = json::parse(idx->mdata);  // https://github.com/nlohmann/json

	if (mdata["ison"] == 0 && !ison) return false;
	
	accounts accountstable( SIMPLEASSETSCONTRACT, _self.value );
	const auto& ac = accountstable.get( FUELTOKENID );
	
	if (ac.balance.amount >=  LAMPUSEFUEL) {
		action saRes1 = action(
			permission_level{get_self(), "active"_n},
			SIMPLEASSETSCONTRACT,
			"burnf"_n,
			std::make_tuple(get_self(), get_self(), asset(LAMPUSEFUEL, TFUEL), std::string("KeroseneLamp lighting"))
		);
		saRes1.send();

		if (ac.balance.amount > LAMPUSEFUEL ) mdata["img"] = lampImg10;
		if (ac.balance.amount > 3000) mdata["img"] = lampImg50;
		if (ac.balance.amount > 6000) mdata["img"] = lampImg100;
		
		if (ison) mdata["ison"] = 1;
	} else {
		mdata["img"] = lampImg0;
		mdata["ison"] = 0;
	}

	// update Lamps (NFT) mdata 
	action saUpdate = action(
		permission_level{get_self(), "active"_n},
		SIMPLEASSETSCONTRACT,
		"update"_n,
		std::make_tuple(get_self(), _cstate.owner, LAMPID, mdata.dump())
	);
	saUpdate.send();
	
	return true;
}


void KeroseneLamp::receiveEOS( name from, name to, asset quantity, string memo ){

	if (to != get_self()) return;
	require_auth( from );

	std::string memosa = "Spark to ignite KeroseneLamp. Plese make sure Lamp is refueled..";
		action saRes1 = action(
		permission_level{get_self(), "active"_n},
		SIMPLEASSETSCONTRACT,
		"issuef"_n,
		std::make_tuple(from, get_self(), asset(1, TSPARK), memosa)
	);
	saRes1.send();
}


void KeroseneLamp::receiveFT( name from, name to, name author, asset quantity, string memo ){
		
	if (to != get_self()) return;
	require_auth( from );

	conf config(_self, _self.value);
	_cstate = config.exists() ? config.get() : global{};

	
	if (quantity.symbol == TSPARK) {
		check ( _cstate.owner == from, "Only current KeroseneLamp owner can ignate only");
		print("spark was burned");
		useFuel(1);
	}
	
	if (quantity.symbol == TFUEL) {
		accounts accountstable( SIMPLEASSETSCONTRACT, _self.value );
		const auto& ac = accountstable.get( FUELTOKENID );
		
		check (!((ac.balance.amount) > 10000), "Fueltank capacity is 100.00 FUEL, you cannot fuel more...");
		check ( _cstate.owner == from, "Refuel is available only for current Lamp owner");
		
		useFuel(0);
	}
	
	checkTicker();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/*
* Creating deferred transaction to call ticker action
*/
void KeroseneLamp::deftx(uint64_t delay ) {
	if (delay<1) 
		delay = tickertime;

	//send new one
	transaction tickerTX{};
	tickerTX.actions.emplace_back( action({get_self(), "active"_n}, _self, "ticker"_n, now()) );
	tickerTX.delay_sec = delay;
	tickerTX.expiration = time_point_sec(now() + 10); 
	uint128_t sender_id = (uint128_t(now()) << 64) | now()-5;
	tickerTX.send(sender_id, _self); 
}