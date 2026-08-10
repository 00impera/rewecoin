// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";

import "./SystemAccess.sol";
import "./ReweCoinToken.sol";
import "./PlayerData.sol";

contract SwapContract is Ownable {

    SystemAccess public access;
    ReweCoinToken public rewe;
    PlayerData public playerData;

    IERC20 public usdc;
    IERC20 public usdt;

    uint256 public reweToUsdRate = 10; // 10 REWE = 1 USD
    uint256 public usdToReweRate = 10; // 1 USD = 10 REWE

    event SwapReweToUSD(address indexed user, uint256 reweAmount, uint256 usdAmount);
    event SwapUSDToRewe(address indexed user, uint256 usdAmount, uint256 reweAmount);

    constructor(
        address _access,
        address _rewe,
        address _playerData,
        address _usdc,
        address _usdt
    ) {
        access = SystemAccess(_access);
        rewe = ReweCoinToken(_rewe);
        playerData = PlayerData(_playerData);
        usdc = IERC20(_usdc);
        usdt = IERC20(_usdt);
    }

    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }

    // ---------------------------------------------------------
    // SWAP REWE → USDC
    // ---------------------------------------------------------
    function swapReweToUSDC(uint256 reweAmount) external {
        require(reweAmount > 0, "Invalid amount");

        // burn REWE from user
        rewe.burn(msg.sender, reweAmount);

        // convert REWE → USD
        uint256 usdAmount = reweAmount / reweToUsdRate;

        // send USDC to user
        usdc.transfer(msg.sender, usdAmount);

        // update PlayerData
        playerData.addRewe(msg.sender, 0 - reweAmount);
        playerData.addUsd(msg.sender, usdAmount);

        emit SwapReweToUSD(msg.sender, reweAmount, usdAmount);
    }

    // ---------------------------------------------------------
    // SWAP REWE → USDT
    // ---------------------------------------------------------
    function swapReweToUSDT(uint256 reweAmount) external {
        require(reweAmount > 0, "Invalid amount");

        rewe.burn(msg.sender, reweAmount);

        uint256 usdAmount = reweAmount / reweToUsdRate;

        usdt.transfer(msg.sender, usdAmount);

        playerData.addRewe(msg.sender, 0 - reweAmount);
        playerData.addUsd(msg.sender, usdAmount);

        emit SwapReweToUSD(msg.sender, reweAmount, usdAmount);
    }

    // ---------------------------------------------------------
    // SWAP REWE → ETH
    // ---------------------------------------------------------
    function swapReweToETH(uint256 reweAmount) external {
        require(reweAmount > 0, "Invalid amount");

        rewe.burn(msg.sender, reweAmount);

        uint256 usdAmount = reweAmount / reweToUsdRate;

        // simplified: 1 ETH = 3000 USD
        uint256 ethAmount = usdAmount / 3000;

        payable(msg.sender).transfer(ethAmount);

        playerData.addRewe(msg.sender, 0 - reweAmount);
        playerData.addUsd(msg.sender, usdAmount);

        emit SwapReweToUSD(msg.sender, reweAmount, usdAmount);
    }

    // ---------------------------------------------------------
    // SWAP USDC → REWE
    // ---------------------------------------------------------
    function swapUSDCToRewe(uint256 usdAmount) external {
        require(usdAmount > 0, "Invalid amount");

        usdc.transferFrom(msg.sender, address(this), usdAmount);

        uint256 reweAmount = usdAmount * usdToReweRate;

        rewe.mint(msg.sender, reweAmount);

        playerData.addUsd(msg.sender, usdAmount);
        playerData.addRewe(msg.sender, reweAmount);

        emit SwapUSDToRewe(msg.sender, usdAmount, reweAmount);
    }

    // ---------------------------------------------------------
    // SWAP USDT → REWE
    // ---------------------------------------------------------
    function swapUSDTToRewe(uint256 usdAmount) external {
        require(usdAmount > 0, "Invalid amount");

        usdt.transferFrom(msg.sender, address(this), usdAmount);

        uint256 reweAmount = usdAmount * usdToReweRate;

        rewe.mint(msg.sender, reweAmount);

        playerData.addUsd(msg.sender, usdAmount);
        playerData.addRewe(msg.sender, reweAmount);

        emit SwapUSDToRewe(msg.sender, usdAmount, reweAmount);
    }

    // ---------------------------------------------------------
    // OWNER SETTINGS
    // ---------------------------------------------------------
    function updateRates(uint256 _reweToUsd, uint256 _usdToRewe) external onlyOwner {
        reweToUsdRate = _reweToUsd;
        usdToReweRate = _usdToRewe;
    }

    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }
}
