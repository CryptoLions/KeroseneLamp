###########################################################################################
###########################################################################################
##
##  KeroseneLamp powerd by SimpleAssets (Digital Assets)
##  (C) 2019 by CryptoLions [ https://CryptoLions.io ]
##
##  Medium Post: https://medium.com/@cryptolions/the-prospectors-lantern-anatomy-of-a-simple-asset-ae0b6cf4f9b
##
##
##  SimpleAssets - simple standard for digital assets (ie. Non-Fungible Tokens).
##                 for EOSIO blockchains
##
##
##    WebSite:        https://simpleassets.io
##    GitHub:         https://github.com/CryptoLions/SimpleAssets.
##    Presentation:   https://medium.com/@cryptolions/introducing-simple-assets-b4e17caafaa4
##
###########################################################################################
###########################################################################################

CLEOS=cleos.sh

ACC=kerosenelamp

#Jungle
KEY=EOS8jPrLzUaEJUNGLeAzp4gUUzRntsBAymiY4zZDpLJTdU43gCoZN

ACCA=simpleassets
ACCM=simplemarket


## Create Simple Assets NFT and FT


#---FT----
# ./$CLEOS push action $ACCA createf '["'$ACC'", "1000000000.00 FUEL", 1, "{\"name\":\"Low Grade Fuel\",\"img\":\"https://i.imgur.com/0JuYEdq.png\",\"memo\":\"\" }"]' -p $ACC
# ./$CLEOS push action $ACCA createf '["'$ACC'", "10000000 SPARK", 1, "{\"name\":\"Spark\",\"img\":\"https://i.imgur.com/KXGfI2Z.png\",\"memo\":\"\" }"]' -p $ACC
# ./$CLEOS push action $ACCA issuef '["'$ACC'", "'$ACC'", "100.00 FUEL", "init"]' -p $ACC


#----NFT---
asset='{"author": "'$ACC'",
        "category": "lamp",
        "owner": "'$ACC'",
        "idata": "by Prospectors.io and CryptoLions.io",
        "mdata": "{\"ison\":1, \"img\": \"\", \"name\": \"Kerosene Lamp\" }",
        "requireclaim" : false
        }'

#./$CLEOS push action $ACCA create "$asset" -p $ACC


-----------------------------------------------------------------


#### Deploy 
## add eosio.code permission to send fdeferred tx

#./$CLEOS set account permission $ACC active '{"threshold": 1,"keys": [{"key": "'$KEY'","weight": 1}],"accounts": [{"permission":{"actor":"'$ACC'","permission":"eosio.code"},"weight":1}]}'
#./$CLEOS set contract $ACC ./build/SimpleAssetsTorchDemo -p $ACC



#### Activate (id from created NFT)

#./cleos.sh push action $ACCA transfer '["'$ACC'", "testertester", [100000000000350], "start"]' -p $ACC



## Give Lamp
#./cleos.sh push action $ACCA offer '["testertester", "bohdanbohdan", [100000000000350], "pass"]' -p testertester

## Return Lamp (if wasn't taken)
#./cleos.sh push action $ACCA canceloffer '["testertester", [100000000000350]]' -p bohdanbohdan

## Take Lamp
#./cleos.sh push action $ACCA claim '["bohdanbohdan", [100000000000350]]' -p bohdanbohdan



# Reinit ticker
#./cleos.sh push action $ACC ticker '[]' -p $ACC


#--------------------------------------

## Get Fuel
#./cleos.sh push action $ACC getfuel '["testertester"]' -p testertester

## Refuel
#./cleos.sh push action $ACCA transferf '["testertester", "'$ACC'", "'$ACC'", "90.00 FUEL", "refuel"]' -p testertester

## Buy Spark
#./cleos.sh transfer testertester $ACC "0.2000 EOS" "for spark"

## Ignate Lamp
#./cleos.sh push action $ACCA transferf '["testertester", "'$ACC'", "'$ACC'", "1 SPARK", "ignite"]' -p testertester


#-------------------------------------------

TABLES 


#./$CLEOS get table $ACCA $ACC sassets -l 1000
#./$CLEOS get table $ACCA $ACC accounts 
#./$CLEOS get table $ACCA $ACC stat


#./$CLEOS get table $ACCA bohdanbohdan sassets -l 1000
#./$CLEOS get table $ACCA testertester sassets -l 1000

#./$CLEOS get table $ACCA bohdanbohdan accounts 
