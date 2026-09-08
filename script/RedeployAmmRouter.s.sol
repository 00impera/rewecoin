// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import {Script, console} from "forge-std/Script.sol";
import {AmmRouter} from "../src/AmmRouter.sol";

contract RedeployAmmRouterScript is Script {
    address constant AMM_FACTORY = 0x74a13c7372f6B34a37e3bb543FBbe73EaB24bF1f;
    address constant WMON_CANONICAL = 0x3bd359C1119dA7Da1D913D1C4D2B7c461115433A;

    function run() external {
        uint256 deployerKey = vm.envUint("PRIVATE_KEY");
        vm.startBroadcast(deployerKey);

        AmmRouter router = new AmmRouter(AMM_FACTORY, WMON_CANONICAL);

        vm.stopBroadcast();

        console.log("=== REDEPLOY AmmRouter (WMON canonic) ===");
        console.log("AmmRouter NOU:", address(router));
        console.log("AmmFactory (nemodificat):", AMM_FACTORY);
        console.log("WMON canonic folosit:", WMON_CANONICAL);
    }
}
