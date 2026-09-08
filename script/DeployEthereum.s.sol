// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import {Script, console} from "forge-std/Script.sol";
import {ReweCoinTokenEthereum} from "../src/ReweCoinTokenEthereum.sol";
import {BridgeEthereum} from "../src/BridgeEthereum.sol";

contract DeployEthereumScript is Script {
    address constant LZ_ENDPOINT_ETHEREUM = 0x1a44076050125825900e736c501f859c50fE728c;

    function run() external {
        uint256 deployerKey = vm.envUint("PRIVATE_KEY");
        address deployer = vm.addr(deployerKey);

        vm.startBroadcast(deployerKey);

        ReweCoinTokenEthereum rewe = new ReweCoinTokenEthereum();
        BridgeEthereum bridge = new BridgeEthereum(LZ_ENDPOINT_ETHEREUM, deployer, address(rewe));

        rewe.setBridge(address(bridge));

        vm.stopBroadcast();

        console.log("=== DEPLOY ETHEREUM COMPLET ===");
        console.log("ReweCoinTokenEthereum: ", address(rewe));
        console.log("BridgeEthereum:        ", address(bridge));
        console.log("Urmeaza: setPeer() reciproc intre BridgeEthereum si BridgeMonad.");
    }
}
