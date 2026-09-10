// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;
import "@openzeppelin/contracts/access/Ownable.sol";
import "./SystemAccess.sol";
import "./Leaderboard.sol";
import "./PlayerData.sol";
contract PoolManager is Ownable {
    SystemAccess public access;
    Leaderboard public leaderboard;
    PlayerData public playerData;
    struct Pool {
        uint256 id;
        uint256 targetUsd;
        uint256 currentUsd;
        uint256 rewardRewe;
        bool active;
        bool completed;
    }
    uint256 public nextPoolId;
    mapping(uint256 => Pool) public pools;
    event PoolCreated(uint256 indexed poolId, uint256 targetUsd, uint256 rewardRewe);
    event PoolUpdated(uint256 indexed poolId, uint256 newUsd);
    event PoolCompleted(uint256 indexed poolId);
    event UserJoinedPool(address indexed user, uint256 indexed poolId, uint256 usdAmount);
    constructor(
        address _access,
        address _leaderboard,
        address _playerData
    ) Ownable(msg.sender) {
        access = SystemAccess(_access);
        leaderboard = Leaderboard(_leaderboard);
        playerData = PlayerData(_playerData);
    }
    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }
    function createPool(uint256 targetUsd, uint256 rewardRewe) external onlyOwner {
        uint256 id = nextPoolId;
        nextPoolId++;
        pools[id] = Pool({
            id: id,
            targetUsd: targetUsd,
            currentUsd: 0,
            rewardRewe: rewardRewe,
            active: true,
            completed: false
        });
        emit PoolCreated(id, targetUsd, rewardRewe);
    }
    function joinPool(address user, uint256 poolId, uint256 usdAmount) external onlySystem {
        Pool storage p = pools[poolId];
        require(p.active, "Pool not active");
        require(!p.completed, "Pool completed");
        p.currentUsd += usdAmount;
        playerData.joinPool(user, poolId);
        leaderboard.updateLeaderboard(poolId, user, p.currentUsd);
        emit UserJoinedPool(user, poolId, usdAmount);
        if (p.currentUsd >= p.targetUsd) {
            p.completed = true;
            p.active = false;
            emit PoolCompleted(poolId);
        }
        emit PoolUpdated(poolId, p.currentUsd);
    }
    function getPool(uint256 poolId)
        external
        view
        returns (
            uint256 id,
            uint256 targetUsd,
            uint256 currentUsd,
            uint256 rewardRewe,
            bool active,
            bool completed
        )
    {
        Pool memory p = pools[poolId];
        return (p.id, p.targetUsd, p.currentUsd, p.rewardRewe, p.active, p.completed);
    }
    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }
    function updateLeaderboard(address newLeaderboard) external onlyOwner {
        leaderboard = Leaderboard(newLeaderboard);
    }
    function updatePlayerData(address newPlayerData) external onlyOwner {
        playerData = PlayerData(newPlayerData);
    }
}
