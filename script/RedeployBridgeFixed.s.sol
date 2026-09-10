// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import {Script, console} from "forge-std/Script.sol";
import {SystemAccess} from "../src/SystemAccess.sol";
import {ReweCoinToken} from "../src/ReweCoinToken.sol";
import {BridgeMonad} from "../src/BridgeMonad.sol";

contract RedeployBridgeFixedScript is Script {
    address constant SYSTEM_ACCESS = 0xC7CFd53C0cC73168549aff18f3a0e5faF4113261;
    address constant REWE_TOKEN    = 0x330ACAD0591FEBBED0Fe7C28dde6704ac00Da9ae;
    address constant LZ_ENDPOINT_V2 = 0x6F475642a6e85809B1c36Fa62763669b1b48DD5B;

    function run() external {
        uint256 deployerKey = vm.envUint("PRIVATE_KEY");
        address deployer = vm.addr(deployerKey);

        vm.startBroadcast(deployerKey);

        BridgeMonad newBridge = new BridgeMonad(LZ_ENDPOINT_V2, deployer, SYSTEM_ACCESS, REWE_TOKEN);

        SystemAccess(SYSTEM_ACCESS).addSystem(address(newBridge));

        vm.stopBroadcast();

        console.log("=== BRIDGEMONAD REDEPLOY (fix interfata reala LayerZero) ===");
        console.log("BridgeMonad NOU: ", address(newBridge));
        console.log("Vechiul BridgeMonad (defect) ramane la: 0xDA80A7c03F269129Da6778584927d00c56262bF3 - IGNORA-L");
    }
}
