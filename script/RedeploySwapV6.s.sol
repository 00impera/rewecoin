// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import {Script, console} from "forge-std/Script.sol";
import {SystemAccess} from "../src/SystemAccess.sol";
import {SwapContract} from "../src/SwapContract.sol";

contract RedeploySwapV6Script is Script {
    address constant SYSTEM_ACCESS      = 0xC7CFd53C0cC73168549aff18f3a0e5faF4113261;
    address constant REWE_TOKEN         = 0x330ACAD0591FEBBED0Fe7C28dde6704ac00Da9ae;
    address constant PLAYER_DATA        = 0xdAEC63459c3a0B6a1E9BA4d51074EDD7462f2b30;
    address constant USDC               = 0x754704Bc059F8C67012fEd69BC8A327a5aafb603;
    address constant USDT0              = 0xe7cd86e13AC4309349F30B3435a9d337750fC82D;
    address constant CHAINLINK_MON_USD  = 0xBcD78f76005B7515837af6b50c7C52BCf73822fb;

    address constant OLD_SWAP_V5        = 0x183DB1B001F3813729FaB7903AbD383b76CF15B9;

    function run() external {
        uint256 deployerKey = vm.envUint("PRIVATE_KEY");
        vm.startBroadcast(deployerKey);

        SwapContract newSwap = new SwapContract(
            SYSTEM_ACCESS,
            REWE_TOKEN,
            PLAYER_DATA,
            USDC,
            USDT0,
            CHAINLINK_MON_USD
        );

        SystemAccess(SYSTEM_ACCESS).addSystem(address(newSwap));
        SystemAccess(SYSTEM_ACCESS).removeSystem(OLD_SWAP_V5);

        vm.stopBroadcast();

        console.log("=== REDEPLOY SwapContract v6 (receive() + withdrawMON) ===");
        console.log("SwapContract NOU (v6):", address(newSwap));
        console.log("SwapContract v5 dezactivat:", OLD_SWAP_V5);
    }
}
