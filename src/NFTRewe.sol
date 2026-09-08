// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;
import "@openzeppelin/contracts/token/ERC721/ERC721.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/utils/Strings.sol";
import "./SystemAccess.sol";
contract NFTRewe is ERC721, Ownable {
    SystemAccess public access;
    enum NFTType { GOLD, ICE, PLATINUM }
    struct NFTData {
        NFTType nftType;
        uint256 poolId;
        uint256 usdValue;
        uint256 timestamp;
    }
    mapping(uint256 => NFTData) public nftInfo;
    uint256 public nextId;
    event MintNFT(address indexed to, uint256 indexed id, NFTType nftType, uint256 poolId, uint256 usdValue);
    event BurnNFT(uint256 indexed id);
    constructor(address _access) ERC721("ReweNFT", "RNFT") Ownable(msg.sender) {
        access = SystemAccess(_access);
    }
    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }
    function mintNFT(
        address to,
        NFTType nftType,
        uint256 poolId,
        uint256 usdValue
    ) external onlySystem returns (uint256) {
        uint256 id = nextId;
        nextId++;
        _safeMint(to, id);
        nftInfo[id] = NFTData({
            nftType: nftType,
            poolId: poolId,
            usdValue: usdValue,
            timestamp: block.timestamp
        });
        emit MintNFT(to, id, nftType, poolId, usdValue);
        return id;
    }
    function burnNFT(uint256 id) external onlySystem {
        _burn(id);
        delete nftInfo[id];
        emit BurnNFT(id);
    }
    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }
    function _imageUrl(NFTType nftType) internal pure returns (string memory) {
        if (nftType == NFTType.GOLD) {
            return "ipfs://QmcaeMa9cFzUusB231BkkW3oTwCLsCgpSkePJF937dKTQW";
        } else if (nftType == NFTType.ICE) {
            return "ipfs://QmVSBip9r849swCct1dbVNxjrucJfWvhzKgweWi7XQSe8X";
        } else {
            return "ipfs://QmQDVAhx7vLuMY9h2jivkoMD5XnVB2B2sC3peJjqvSv5BW";
        }
    }
    function tokenURI(uint256 id) public view override returns (string memory) {
        require(_ownerOf(id) != address(0), "NFT does not exist");
        NFTData memory data = nftInfo[id];
        string memory nftTypeStr = data.nftType == NFTType.GOLD
            ? "Gold"
            : data.nftType == NFTType.ICE
                ? "Ice"
                : "Platinum";
        return string(
            abi.encodePacked(
                "data:application/json;utf8,{",
                '"name":"ReweNFT #', _toString(id), '",',
                '"description":"Dynamic NFT for ReweCoin ecosystem.",',
                '"image":"', _imageUrl(data.nftType), '",',
                '"attributes":[',
                    '{"trait_type":"Type","value":"', nftTypeStr, '"},',
                    '{"trait_type":"Pool","value":"', _toString(data.poolId), '"},',
                    '{"trait_type":"USD Value","value":"', _toString(data.usdValue), '"},',
                    '{"trait_type":"Timestamp","value":"', _toString(data.timestamp), '"}',
                "]}"
            )
        );
    }
    function _toString(uint256 value) internal pure returns (string memory) {
        return Strings.toString(value);
    }
}
