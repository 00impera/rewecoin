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

    uint256 public constant USD_DECIMALS_SCALE = 1e12;

    event SwapReweToUSD(address indexed user, uint256 reweAmount, uint256 usdAmountRaw);
    event SwapUSDToRewe(address indexed user, uint256 usdAmountRaw, uint256 reweAmount);

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

    function swapReweToUSDC(uint256 reweAmount) external {
        require(reweAmount > 0, "Invalid amount");
        rewe.burn(msg.sender, reweAmount);
        uint256 usdAmount18 = reweAmount / reweToUsdRate;
        uint256 usdAmountRaw = usdAmount18 / USD_DECIMALS_SCALE;
        usdc.transfer(msg.sender, usdAmountRaw);
        playerData.subRewe(msg.sender, reweAmount);
        playerData.addUsd(msg.sender, usdAmount18);
        emit SwapReweToUSD(msg.sender, reweAmount, usdAmountRaw);
    }

    function swapReweToUSDT(uint256 reweAmount) external {
        require(reweAmount > 0, "Invalid amount");
        rewe.burn(msg.sender, reweAmount);
        uint256 usdAmount18 = reweAmount / reweToUsdRate;
        uint256 usdAmountRaw = usdAmount18 / USD_DECIMALS_SCALE;
        usdt.transfer(msg.sender, usdAmountRaw);
        playerData.subRewe(msg.sender, reweAmount);
        playerData.addUsd(msg.sender, usdAmount18);
        emit SwapReweToUSD(msg.sender, reweAmount, usdAmountRaw);
    }

    function swapReweToETH(uint256 reweAmount) external {
        require(reweAmount > 0, "Invalid amount");
        rewe.burn(msg.sender, reweAmount);
        uint256 usdAmount18 = reweAmount / reweToUsdRate;
        uint256 ethAmount = usdAmount18 / 3000;
        payable(msg.sender).transfer(ethAmount);
        playerData.subRewe(msg.sender, reweAmount);
        playerData.addUsd(msg.sender, usdAmount18);
        emit SwapReweToUSD(msg.sender, reweAmount, ethAmount);
    }

    function swapUSDCToRewe(uint256 usdAmountRaw) external {
        require(usdAmountRaw > 0, "Invalid amount");
        usdc.transferFrom(msg.sender, address(this), usdAmountRaw);
        uint256 usdAmount18 = usdAmountRaw * USD_DECIMALS_SCALE;
        uint256 reweAmount = usdAmount18 * usdToReweRate;
        rewe.mint(msg.sender, reweAmount);
        playerData.addUsd(msg.sender, usdAmount18);
        playerData.addRewe(msg.sender, reweAmount);
        emit SwapUSDToRewe(msg.sender, usdAmountRaw, reweAmount);
    }

    function swapUSDTToRewe(uint256 usdAmountRaw) external {
        require(usdAmountRaw > 0, "Invalid amount");
        usdt.transferFrom(msg.sender, address(this), usdAmountRaw);
        uint256 usdAmount18 = usdAmountRaw * USD_DECIMALS_SCALE;
        uint256 reweAmount = usdAmount18 * usdToReweRate;
        rewe.mint(msg.sender, reweAmount);
        playerData.addUsd(msg.sender, usdAmount18);
        playerData.addRewe(msg.sender, reweAmount);
        emit SwapUSDToRewe(msg.sender, usdAmountRaw, reweAmount);
    }

    function updateRates(uint256 _reweToUsd, uint256 _usdToRewe) external onlyOwner {
        reweToUsdRate = _reweToUsd;
        usdToReweRate = _usdToRewe;
    }

    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }
}
