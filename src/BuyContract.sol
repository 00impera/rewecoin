// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";
import "@openzeppelin/contracts/token/ERC20/utils/SafeERC20.sol";
import "@chainlink/contracts/src/v0.8/shared/interfaces/AggregatorV3Interface.sol";

import "./SystemAccess.sol";
import "./ReweCoinToken.sol";
import "./NFTRewe.sol";
import "./PlayerData.sol";
import "./PoolManager.sol";

contract BuyContract is Ownable {
    using SafeERC20 for IERC20;

    SystemAccess public access;
    ReweCoinToken public rewe;
    NFTRewe public nft;
    PlayerData public playerData;
    PoolManager public poolManager;

    IERC20 public usdc;
    IERC20 public usdt;

    // USDC/USDT reale pe mainnet au 6 zecimale (confirmat on-chain).
    // usdAmount intern e normalizat mereu la 18 zecimale, ca sa fie
    // consistent cu usdAmount calculat pe calea buyWithMON (Chainlink 8 zecimale + MON 18 zecimale).
    uint256 private constant USD_DECIMALS_SCALE = 1e12; // 10^(18-6)

    AggregatorV3Interface public monUsdPriceFeed;
    uint256 public constant PRICE_FEED_MAX_AGE = 3600;

    uint256 public usdToReweRate = 10;

    event BuyUSD(address indexed user, uint256 usdAmount, uint256 poolId);
    event BuyREWE(address indexed user, uint256 reweAmount);
    event MintNFT(address indexed user, uint256 nftId);
    event WithdrawMON(address indexed to, uint256 amount);

    constructor(
        address _access,
        address _rewe,
        address _nft,
        address _playerData,
        address _poolManager,
        address _usdc,
        address _usdt,
        address _monUsdPriceFeed
    ) Ownable(msg.sender) {
        access = SystemAccess(_access);
        rewe = ReweCoinToken(_rewe);
        nft = NFTRewe(_nft);
        playerData = PlayerData(_playerData);
        poolManager = PoolManager(_poolManager);
        usdc = IERC20(_usdc);
        usdt = IERC20(_usdt);
        monUsdPriceFeed = AggregatorV3Interface(_monUsdPriceFeed);
    }

    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }

    function buyWithUSDC(uint256 amount, uint256 poolId) external {
        require(amount > 0, "Invalid amount");
        usdc.safeTransferFrom(msg.sender, address(this), amount);
        uint256 usdAmount = amount * USD_DECIMALS_SCALE;
        _processBuy(msg.sender, usdAmount, poolId);
    }

    function buyWithUSDT(uint256 amount, uint256 poolId) external {
        require(amount > 0, "Invalid amount");
        usdt.safeTransferFrom(msg.sender, address(this), amount);
        uint256 usdAmount = amount * USD_DECIMALS_SCALE;
        _processBuy(msg.sender, usdAmount, poolId);
    }

    function buyWithMON(uint256 poolId) external payable {
        require(msg.value > 0, "Invalid MON");
        uint256 usdAmount = _getUsdValueOfMon(msg.value);
        _processBuy(msg.sender, usdAmount, poolId);
    }

    function _getUsdValueOfMon(uint256 monAmount) internal view returns (uint256) {
        (, int256 price, , uint256 updatedAt, ) = monUsdPriceFeed.latestRoundData();
        require(price > 0, "Invalid price feed");
        require(block.timestamp - updatedAt < PRICE_FEED_MAX_AGE, "Stale price feed");
        return (monAmount * uint256(price)) / 1e8;
    }

    function _processBuy(address user, uint256 usdAmount, uint256 poolId) internal {
        playerData.addUsd(user, usdAmount);
        uint256 reweAmount = usdAmount * usdToReweRate;
        rewe.mint(user, reweAmount);
        playerData.addRewe(user, reweAmount);
        emit BuyUSD(user, usdAmount, poolId);
        emit BuyREWE(user, reweAmount);
        uint256 nftId = nft.mintNFT(user, NFTRewe.NFTType.GOLD, poolId, usdAmount);
        playerData.addNFT(user, nftId);
        emit MintNFT(user, nftId);
        poolManager.joinPool(user, poolId, usdAmount);
    }

    function updateRate(uint256 newRate) external onlyOwner {
        usdToReweRate = newRate;
    }

    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }

    function updateNFT(address newNft) external onlyOwner {
        nft = NFTRewe(newNft);
    }

    function updatePriceFeed(address newFeed) external onlyOwner {
        monUsdPriceFeed = AggregatorV3Interface(newFeed);
    }

    function withdrawMON(address payable to) external onlyOwner {
        uint256 balance = address(this).balance;
        require(balance > 0, "No MON to withdraw");
        (bool success, ) = to.call{value: balance}("");
        require(success, "Withdraw failed");
        emit WithdrawMON(to, balance);
    }
}
