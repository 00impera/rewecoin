// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";

import "./SystemAccess.sol";
import "./ReweCoinToken.sol";
import "./NFTRewe.sol";
import "./PlayerData.sol";
import "./PoolManager.sol";

contract BuyContract is Ownable {

    SystemAccess public access;
    ReweCoinToken public rewe;
    NFTRewe public nft;
    PlayerData public playerData;
    PoolManager public poolManager;

    IERC20 public usdc;
    IERC20 public usdt;

    uint256 public usdToReweRate = 10; // 1 USD = 10 REWE

    event BuyUSD(address indexed user, uint256 usdAmount, uint256 poolId);
    event BuyREWE(address indexed user, uint256 reweAmount);
    event MintNFT(address indexed user, uint256 nftId);

    constructor(
        address _access,
        address _rewe,
        address _nft,
        address _playerData,
        address _poolManager,
        address _usdc,
        address _usdt
    ) {
        access = SystemAccess(_access);
        rewe = ReweCoinToken(_rewe);
        nft = NFTRewe(_nft);
        playerData = PlayerData(_playerData);
        poolManager = PoolManager(_poolManager);
        usdc = IERC20(_usdc);
        usdt = IERC20(_usdt);
    }

    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }

    // ---------------------------------------------------------
    // BUY WITH USDC
    // ---------------------------------------------------------
    function buyWithUSDC(uint256 amount, uint256 poolId) external {
        require(amount > 0, "Invalid amount");

        usdc.transferFrom(msg.sender, address(this), amount);

        uint256 usdAmount = amount; // 1 USDC = 1 USD

        _processBuy(msg.sender, usdAmount, poolId);
    }

    // ---------------------------------------------------------
    // BUY WITH USDT
    // ---------------------------------------------------------
    function buyWithUSDT(uint256 amount, uint256 poolId) external {
        require(amount > 0, "Invalid amount");

        usdt.transferFrom(msg.sender, address(this), amount);

        uint256 usdAmount = amount; // 1 USDT = 1 USD

        _processBuy(msg.sender, usdAmount, poolId);
    }

    // ---------------------------------------------------------
    // BUY WITH ETH
    // ---------------------------------------------------------
    function buyWithETH(uint256 poolId) external payable {
        require(msg.value > 0, "Invalid ETH");

        // Simplified conversion: 1 ETH = 3000 USD (example)
        uint256 usdAmount = msg.value * 3000;

        _processBuy(msg.sender, usdAmount, poolId);
    }

    // ---------------------------------------------------------
    // INTERNAL BUY LOGIC
    // ---------------------------------------------------------
    function _processBuy(address user, uint256 usdAmount, uint256 poolId) internal {

        // 1. Update PlayerData
        playerData.addUsd(user, usdAmount);

        // 2. Convert USD → REWE
        uint256 reweAmount = usdAmount * usdToReweRate;
        rewe.mint(user, reweAmount);
        playerData.addRewe(user, reweAmount);

        emit BuyUSD(user, usdAmount, poolId);
        emit BuyREWE(user, reweAmount);

        // 3. Mint NFT
        uint256 nftId = nft.mintNFT(
            user,
            NFTRewe.NFTType.GOLD,
            poolId,
            usdAmount
        );

        playerData.addNFT(user, nftId);
        emit MintNFT(user, nftId);

        // 4. Join pool
        poolManager.joinPool(user, poolId, usdAmount);
    }

    // ---------------------------------------------------------
    // OWNER SETTINGS
    // ---------------------------------------------------------
    function updateRate(uint256 newRate) external onlyOwner {
        usdToReweRate = newRate;
    }

    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }
}
