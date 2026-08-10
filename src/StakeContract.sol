// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/access/Ownable.sol";
import "./SystemAccess.sol";
import "./ReweCoinToken.sol";
import "./PlayerData.sol";

contract StakeContract is Ownable {

    SystemAccess public access;
    ReweCoinToken public rewe;
    PlayerData public playerData;

    struct StakeInfo {
        uint256 amount;
        uint256 startTime;
        bool active;
    }

    mapping(address => StakeInfo) public stakes;

    uint256 public pointsPerHour = 5; // example: 5 points per hour

    event Staked(address indexed user, uint256 amount);
    event Unstaked(address indexed user, uint256 amount, uint256 pointsEarned);

    constructor(
        address _access,
        address _rewe,
        address _playerData
    ) {
        access = SystemAccess(_access);
        rewe = ReweCoinToken(_rewe);
        playerData = PlayerData(_playerData);
    }

    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }

    // ---------------------------------------------------------
    // STAKE REWE
    // ---------------------------------------------------------
    function stake(uint256 amount) external {
        require(amount > 0, "Invalid amount");
        require(!stakes[msg.sender].active, "Already staking");

        // transfer REWE from user to contract
        rewe.transferFrom(msg.sender, address(this), amount);

        stakes[msg.sender] = StakeInfo({
            amount: amount,
            startTime: block.timestamp,
            active: true
        });

        emit Staked(msg.sender, amount);
    }

    // ---------------------------------------------------------
    // UNSTAKE REWE
    // ---------------------------------------------------------
    function unstake() external {
        StakeInfo storage s = stakes[msg.sender];
        require(s.active, "Not staking");

        uint256 stakedAmount = s.amount;

        // calculate points
        uint256 hoursStaked = (block.timestamp - s.startTime) / 3600;
        uint256 pointsEarned = hoursStaked * pointsPerHour;

        // reset stake
        s.active = false;
        s.amount = 0;

        // return REWE to user
        rewe.transfer(msg.sender, stakedAmount);

        // add points to PlayerData
        playerData.addPoints(msg.sender, pointsEarned);

        emit Unstaked(msg.sender, stakedAmount, pointsEarned);
    }

    // ---------------------------------------------------------
    // OWNER SETTINGS
    // ---------------------------------------------------------
    function updatePointsRate(uint256 newRate) external onlyOwner {
        pointsPerHour = newRate;
    }

    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }
}
