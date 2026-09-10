// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import {Script, console} from "forge-std/Script.sol";
import {SystemAccess} from "../src/SystemAccess.sol";
import {ReweCoinToken} from "../src/ReweCoinToken.sol";
import {PlayerData} from "../src/PlayerData.sol";
import {PoolManager} from "../src/PoolManager.sol";
import {BuyContract} from "../src/BuyContract.sol";
import {BridgeMonad} from "../src/BridgeMonad.sol";
import {MockPriceFeed} from "../src/MockPriceFeed.sol";

contract RedeployBuyAndBridgeScript is Script {
    address constant SYSTEM_ACCESS = 0xBb5F0beCB8c886703d06457B866Cf141598893A8;
    address constant REWE_TOKEN    = 0x4DF13861Cb4c43A7662f44F69acb21dD1bC303a0;
    address constant PLAYER_DATA   = 0xA12c714C46dc577E420000244C21a86Fd20A1147;
    address constant POOL_MANAGER  = 0x837a17F38D0fEBf318912aF165A90550AA12720a;
    address constant NEW_NFT       = 0x70e3B3DfA62dF204d67b71D8C5bb4e962E9Efd6F;
    address constant MOCK_USDC     = 0x7D1f79d2a88C5a86e6Cc663b9Abd146aa8A53E3c;
    address constant MOCK_USDT     = 0x86E30b822E2b0204b7d3955C0B9FF695e3090683;

    // placeholder - bridge complet e amanat pentru faza 2
    address constant LZ_ENDPOINT_PLACEHOLDER = address(uint160(0xdEaD));

    function run() external {
        uint256 deployerKey = vm.envUint("PRIVATE_KEY");
        vm.startBroadcast(deployerKey);

        MockPriceFeed monUsdFeed = new MockPriceFeed(3000 * 1e8); // 1 MON =  pt test

        BuyContract newBuyContract = new BuyContract(
            SYSTEM_ACCESS,
            REWE_TOKEN,
            NEW_NFT,
            PLAYER_DATA,
            POOL_MANAGER,
            MOCK_USDC,
            MOCK_USDT,
            address(monUsdFeed)
        );

        BridgeMonad newBridge = new BridgeMonad(LZ_ENDPOINT_PLACEHOLDER, vm.addr(deployerKey), SYSTEM_ACCESS, REWE_TOKEN);

        SystemAccess(SYSTEM_ACCESS).addSystem(address(newBuyContract));
        SystemAccess(SYSTEM_ACCESS).addSystem(address(newBridge));

        vm.stopBroadcast();

        console.log("=== REDEPLOY BUY + BRIDGE COMPLET ===");
        console.log("BuyContract NOU:  ", address(newBuyContract));
        console.log("BridgeMonad NOU:  ", address(newBridge));
        console.log("MockPriceFeed:    ", address(monUsdFeed));
    }
}
