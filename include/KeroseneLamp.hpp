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

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/singleton.hpp>

using namespace eosio;
using std::string;


#define TFUEL symbol("FUEL", 2)  
#define TSPARK symbol("SPARK", 0)

name SIMPLEASSETSCONTRACT = "simpleassets"_n;	//SimpleAssest account
name BONUSFGCONTRACT = "prospectorsa"_n;	//Author of bonus FT


//Jungle
const uint64_t BONUS_FTID = 100000000000024; 
const uint64_t LAMPID = 100000000000350;
const uint64_t FUELTOKENID = 100000000000348;
asset BONUS_QUANTITY = asset{1, symbol("PTS", 0)};

//MAinnet
//const uint64_t BONUS_FTID = 100000000000083;
//const uint64_t LAMPID = 100000000000293;
//const uint64_t FUELTOKENID = 100000000000291;
//asset BONUS_QUANTITY = asset{1, symbol("PTF", 0)};


const int tickertime = 60*5;  // how often to check/use fuel while burning
const int LAMPUSEFUEL = 200;  // How much fuel it use on tick; 200 = 2.00FUEL ; Full tank 100.00 FUEL; 

//Different Lamp state images
string imgUrl = "https://prospectors.io/auction/img/lamp/";
string lampImg100 = imgUrl+"lamp_b.gif";
string lampImg50 = imgUrl+"lamp_m.gif";
string lampImg10 = imgUrl+"lamp_s.gif";
string lampImg0 = imgUrl+"lamp_n.png";

	
CONTRACT KeroseneLamp : public contract {
	public:
		using contract::contract;

		// Action which listen SimpleAssets events
		ACTION saecreate( name owner, uint64_t assetid );
		using saecreate_action = action_wrapper<"saecreate"_n, &KeroseneLamp::saecreate>;

		ACTION saetransfer( name from, name to, std::vector<uint64_t>& assetids, string memo );
		using saetransfer_action = action_wrapper<"saetransfer"_n, &KeroseneLamp::saetransfer>;

		ACTION saeclaim( name who, std::map< uint64_t, name >& assetids);
		using saeclaim_action = action_wrapper<"saeclaim"_n, &KeroseneLamp::saeclaim>;

		//ACTION saeburn( name who, std::vector<uint64_t>& assetids, string memo  );
		//using saeburn_action = action_wrapper<"saeburn"_n, &KeroseneLamp::saeburn>;


		//Get fueal action. Available only for current owner of the Lamp. Up to max 100.00FUEL (free)
		ACTION getfuel( name acc );
		using getfuel_action = action_wrapper<"getfuel"_n, &KeroseneLamp::getfuel>;

		//ticker action (can be called outside )
		ACTION ticker();
		using ticker_action = action_wrapper<"ticker"_n, &KeroseneLamp::ticker>;



		//on eosio.token::transfer notification (any amount of tokens) to KeroseneLamp, 
		//sender will get 1 spark to ignate a lamp (SPARK can be used if lamp stopped shining)
		//available only for lamp owner.
		void receiveEOS( name from, name to, asset quantity, string memo );

		//on simpleassets::transferf notification In case of FUELL token - 
		//Lamp will be Refueled, in case of SPARK token - Lamp will ignate
		void receiveFT( name from, name to, name author, asset quantity, string memo );

	private:
		//Use Lamp gas by burning FUELL token from kerosenelamp contract. 
		//Also update Lamp NFT mdata: new img (different size of lamp fire) and lamp status `ison` (0 - no fuel stoped shining).
		//ison parametr will not burn lamp after refueling without SPARK..
		bool useFuel(bool ison);

		// Check if start ticker (in case if planned deffered was failed)
		void checkTicker();

		// internal action for ticker
		void tick();

		// creating deferred transaction
		void deftx(uint64_t delay );

		// Table to track last owner, last lamp pass time and last ticker time
		TABLE global {
		global(){}
		name owner;
		uint64_t passtime = 0;
		uint64_t lastticker = 0;

		EOSLIB_SERIALIZE( global, (owner)(lastticker)(lastticker) )
		};

		typedef eosio::singleton< "global"_n, global> conf;
		global _cstate;


		// Table to track who already passed the Lamp and recevied Bonus FT
		TABLE pass {
			name		from;
			name		to;			
			uint64_t	pdate;

			uint64_t primary_key()const { 
				return from.value;
			}
		};
		typedef eosio::multi_index< "passes"_n, pass > passes;		


		//SimpleAssets accounts and sasset structures to work with data.
		TABLE account {
			uint64_t	id;
			uint64_t	author;			
			asset		balance;

			uint64_t primary_key()const { return id; }
		};
		typedef eosio::multi_index< "accounts"_n, account > accounts;		



		TABLE sasset {
			uint64_t				id;
			name					owner;
			name					author;
			name					category;
			string					idata;
			string					mdata;
			std::vector<sasset>		container;
			std::vector<account>	containerf;

			auto primary_key() const { return id; }
			uint64_t by_author() const { return author.value; }
		};

		typedef eosio::multi_index< "sassets"_n, sasset, 		
		eosio::indexed_by< "author"_n, eosio::const_mem_fun<sasset, uint64_t, &sasset::by_author> >
		> sassets;
  
};


//Listen to notifications
extern "C"
void apply(uint64_t receiver, uint64_t code, uint64_t action){
	
	if (code == "eosio.token"_n.value && action == "transfer"_n.value) {
		eosio::execute_action( eosio::name(receiver), eosio::name(code), &KeroseneLamp::receiveEOS );
	} else if (code == SIMPLEASSETSCONTRACT.value && action == "transferf"_n.value) {
		eosio::execute_action( eosio::name(receiver), eosio::name(code), &KeroseneLamp::receiveFT );
	} else if (code == receiver) {
		switch (action) {
			EOSIO_DISPATCH_HELPER(KeroseneLamp, (saecreate)(saetransfer)(saeclaim)(ticker)(getfuel) )
		}
	}
}
