// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";
import "@openzeppelin/contracts/token/ERC20/utils/SafeERC20.sol";
import "@chainlink/contracts/src/v0.8/shared/interfaces/AggregatorV3Interface.sol";

import "./SystemAccess.sol";
import "./ReweCoinToken.sol";
import "./PlayerData.sol";

contract SwapContract is Ownable {
    using SafeERC20 for IERC20;

    SystemAccess public access;
    ReweCoinToken public rewe;
    PlayerData public playerData;

    IERC20 public usdc;
    IERC20 public usdt;

    AggregatorV3Interface public monUsdPriceFeed;
    uint256 public constant PRICE_FEED_MAX_AGE = 3600;

    uint256 public reweToUsdRate = 10;
    uint256 public usdToReweRate = 10;

    uint256 private constant USD_DECIMALS_SCALE = 1e12;

    event SwapReweToUSD(address indexed user, uint256 reweAmount, uint256 usdAmount);
    event SwapUSDToRewe(address indexed user, uint256 usdAmount, uint256 reweAmount);

    event SwapExecuted(
        address indexed trader,
        bool reweIn,
        uint256 amountIn,
        uint256 amountOut,
        uint256 priceAtSwap,
        uint256 timestamp
    );

    event RateUpdated(
        uint256 reweToUsdRate,
        uint256 usdToReweRate,
        uint256 timestamp
    );

    constructor(
        address _access,
        address _rewe,
        address _playerData,
        address _usdc,
        address _usdt,
        address _monUsdPriceFeed
    ) Ownable(msg.sender) {
        access = SystemAccess(_access);
        rewe = ReweCoinToken(_rewe);
        playerData = PlayerData(_playerData);
        usdc = IERC20(_usdc);
        usdt = IERC20(_usdt);
        monUsdPriceFeed = AggregatorV3Interface(_monUsdPriceFeed);
    }

    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }

    receive() external payable {}

    function swapReweToUSDC(uint256 reweAmount) external {
        require(reweAmount > 0, "Invalid amount");

        rewe.burn(msg.sender, reweAmount);

        uint256 usdAmount = reweAmount / reweToUsdRate;

        playerData.addUsd(msg.sender, usdAmount);

        emit SwapReweToUSD(msg.sender, reweAmount, usdAmount);
        emit SwapExecuted(
            msg.sender,
            true,
            reweAmount,
            usdAmount,
            (usdAmount * 1e18) / reweAmount,
            block.timestamp
        );

        usdc.safeTransfer(msg.sender, usdAmount / USD_DECIMALS_SCALE);
    }

    function swapReweToUSDT(uint256 reweAmount) external {
        require(reweAmount > 0, "Invalid amount");

        rewe.burn(msg.sender, reweAmount);

        uint256 usdAmount = reweAmount / reweToUsdRate;

        playerData.addUsd(msg.sender, usdAmount);

        emit SwapReweToUSD(msg.sender, reweAmount, usdAmount);
        emit SwapExecuted(
            msg.sender,
            true,
            reweAmount,
            usdAmount,
            (usdAmount * 1e18) / reweAmount,
            block.timestamp
        );

        usdt.safeTransfer(msg.sender, usdAmount / USD_DECIMALS_SCALE);
    }

    function swapReweToMON(uint256 reweAmount) external {
        require(reweAmount > 0, "Invalid amount");

        rewe.burn(msg.sender, reweAmount);

        uint256 usdAmount = reweAmount / reweToUsdRate;

        uint256 monAmount = _getMonValueOfUsd(usdAmount);

        playerData.addUsd(msg.sender, usdAmount);

        emit SwapReweToUSD(msg.sender, reweAmount, usdAmount);
        emit SwapExecuted(
            msg.sender,
            true,
            reweAmount,
            usdAmount,
            (usdAmount * 1e18) / reweAmount,
            block.timestamp
        );

        (bool success, ) = payable(msg.sender).call{value: monAmount}("");
        require(success, "MON transfer failed");
    }

    function _getMonValueOfUsd(uint256 usdAmount) internal view returns (uint256) {
        (, int256 price, , uint256 updatedAt, ) = monUsdPriceFeed.latestRoundData();
        require(price > 0, "Invalid price feed");
        require(block.timestamp - updatedAt < PRICE_FEED_MAX_AGE, "Stale price feed");
        return (usdAmount * 1e8) / uint256(price);
    }

    function swapUSDCToRewe(uint256 amountRaw) external {
        require(amountRaw > 0, "Invalid amount");

        usdc.safeTransferFrom(msg.sender, address(this), amountRaw);

        uint256 usdAmount = amountRaw * USD_DECIMALS_SCALE;
        uint256 reweAmount = usdAmount * usdToReweRate;

        rewe.mint(msg.sender, reweAmount);

        playerData.addUsd(msg.sender, usdAmount);
        playerData.addRewe(msg.sender, reweAmount);

        emit SwapUSDToRewe(msg.sender, usdAmount, reweAmount);
        emit SwapExecuted(
            msg.sender,
            false,
            usdAmount,
            reweAmount,
            (usdAmount * 1e18) / reweAmount,
            block.timestamp
        );
    }

    function swapUSDTToRewe(uint256 amountRaw) external {
        require(amountRaw > 0, "Invalid amount");

        usdt.safeTransferFrom(msg.sender, address(this), amountRaw);

        uint256 usdAmount = amountRaw * USD_DECIMALS_SCALE;
        uint256 reweAmount = usdAmount * usdToReweRate;

        rewe.mint(msg.sender, reweAmount);

        playerData.addUsd(msg.sender, usdAmount);
        playerData.addRewe(msg.sender, reweAmount);

        emit SwapUSDToRewe(msg.sender, usdAmount, reweAmount);
        emit SwapExecuted(
            msg.sender,
            false,
            usdAmount,
            reweAmount,
            (usdAmount * 1e18) / reweAmount,
            block.timestamp
        );
    }

    function updateRates(uint256 _reweToUsd, uint256 _usdToRewe) external onlyOwner {
        reweToUsdRate = _reweToUsd;
        usdToReweRate = _usdToRewe;
        emit RateUpdated(_reweToUsd, _usdToRewe, block.timestamp);
    }

    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }

    function updatePriceFeed(address newFeed) external onlyOwner {
        monUsdPriceFeed = AggregatorV3Interface(newFeed);
    }

    event WithdrawMON(address indexed to, uint256 amount);

    function withdrawMON(address payable to) external onlyOwner {
        uint256 balance = address(this).balance;
        require(balance > 0, "No MON to withdraw");
        (bool success, ) = to.call{value: balance}("");
        require(success, "Withdraw failed");
        emit WithdrawMON(to, balance);
    }
}
