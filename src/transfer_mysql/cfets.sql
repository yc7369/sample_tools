-- MySQL dump 10.13  Distrib 8.0.19, for Win64 (x86_64)
--
-- Host: 172.24.13.178    Database: yctest
-- ------------------------------------------------------
-- Server version	8.0.23

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `MarketDataXbondDepth`
--
SET FOREIGN_KEY_CHECKS = 0;
DROP TABLE IF EXISTS `MarketDataXbondDepth`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `MarketDataXbondDepth` (
  `hjcode` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `sourceType` int DEFAULT NULL,
  `timems` bigint DEFAULT NULL,
  `recvms` bigint DEFAULT NULL,
  `sn` bigint DEFAULT NULL,
  `marketChan` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `BeginString` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgType` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgSeqNum` int DEFAULT NULL,
  `SendingTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MDReqID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MDBookType` int DEFAULT NULL,
  `SecurityID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MarketDepth` int DEFAULT NULL,
  `TransactTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `OrginData` varbinary(4096) DEFAULT NULL,
  `JsonData` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  INDEX `timems`(`timems`, `sn`) USING BTREE,
  KEY `hjcode` (`hjcode`,`sourceType`,`timems`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MarketDataXbondDepth`
--

LOCK TABLES `MarketDataXbondDepth` WRITE;
/*!40000 ALTER TABLE `MarketDataXbondDepth` DISABLE KEYS */;
/*!40000 ALTER TABLE `MarketDataXbondDepth` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `MarketDataBrokerExecute`
--

DROP TABLE IF EXISTS `MarketDataBrokerExecute`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `MarketDataBrokerExecute` (
  `hjcode` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `sourceType` int DEFAULT NULL,
  `timems` bigint DEFAULT NULL,
  `recvms` bigint DEFAULT NULL,
  `sn` bigint DEFAULT NULL,
  `marketChan` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `BeginString` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgType` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgSeqNum` int DEFAULT NULL,
  `SendingTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MDBookType` int DEFAULT NULL,
  `MDSubBookType` int DEFAULT NULL,
  `SecurityID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `Symbol` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `TermToMaturity` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `CFETSValuationYield` float DEFAULT NULL,
  `Broker` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MarketID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `LatestSubjectRating` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `TradeDate` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `TradeTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `TransactTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `TransactionMethod` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `ExecType` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `ExecID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `OrginData` varbinary(4096) DEFAULT NULL,
  `JsonData` varchar(4096) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  INDEX `timems`(`timems`, `sn`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MarketDataBrokerExecute`
--

LOCK TABLES `MarketDataBrokerExecute` WRITE;
/*!40000 ALTER TABLE `MarketDataBrokerExecute` DISABLE KEYS */;
/*!40000 ALTER TABLE `MarketDataBrokerExecute` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `MarketDataMakingDepth`
--

DROP TABLE IF EXISTS `MarketDataMakingDepth`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `MarketDataMakingDepth` (
  `hjcode` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `sourceType` int DEFAULT NULL,
  `timems` bigint DEFAULT NULL,
  `recvms` bigint DEFAULT NULL,
  `sn` bigint DEFAULT NULL,
  `marketChan` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `BeginString` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgType` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgSeqNum` int DEFAULT NULL,
  `SendingTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MDBookType` int DEFAULT NULL,
  `MDSubBookType` int DEFAULT NULL,
  `MarketIndicator` int DEFAULT NULL,
  `MDReqID` varchar(256) COLLATE utf8mb4_bin DEFAULT NULL,
  `SecurityType` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `SecurityID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `Symbol` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MarketDepth` int DEFAULT NULL,
  `OrginData` varbinary(4096) DEFAULT NULL,
  `JsonData` varchar(256) COLLATE utf8mb4_bin DEFAULT NULL,
  INDEX `timems`(`timems`, `sn`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MarketDataMakingDepth`
--

LOCK TABLES `MarketDataMakingDepth` WRITE;
/*!40000 ALTER TABLE `MarketDataMakingDepth` DISABLE KEYS */;
/*!40000 ALTER TABLE `MarketDataMakingDepth` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `MarketDataXSwapExcute`
--

DROP TABLE IF EXISTS `MarketDataXSwapExcute`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `MarketDataXSwapExcute` (
  `hjcode` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `sourceType` int DEFAULT NULL,
  `timems` bigint DEFAULT NULL,
  `recvms` bigint DEFAULT NULL,
  `sn` bigint DEFAULT NULL,
  `marketChan` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `BeginString` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgType` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgSeqNum` int DEFAULT NULL,
  `SendingTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MarketIndicator` int DEFAULT NULL,
  `MDBookType` int DEFAULT NULL,
  `TransactTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `TransactionMethod` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `TradeMethod` int DEFAULT NULL,
  `RealTimeUndertakeFlag` int DEFAULT NULL,
  `BridgeDealIndic` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `SplitIndicator` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `SecurityID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `OrginData`  varbinary(4096) DEFAULT NULL,
  `JsonData` varchar(4096) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  INDEX `timems`(`timems`, `sn`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MarketDataXSwapExcute`
--

LOCK TABLES `MarketDataXSwapExcute` WRITE;
/*!40000 ALTER TABLE `MarketDataXSwapExcute` DISABLE KEYS */;
/*!40000 ALTER TABLE `MarketDataXSwapExcute` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `MarketDataXSwapDepth`
--

DROP TABLE IF EXISTS `MarketDataXSwapDepth`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `MarketDataXSwapDepth` (
  `hjcode` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `sourceType` int DEFAULT NULL,
  `timems` bigint DEFAULT NULL,
  `recvms` bigint DEFAULT NULL,
  `sn` bigint DEFAULT NULL,
  `marketChan` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `BeginString` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgType` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgSeqNum` int DEFAULT NULL,
  `SendingTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MarketIndicator` int DEFAULT NULL,
  `MDBookType` int DEFAULT NULL,
  `RealTimeUndertakeFlag` int DEFAULT NULL,
  `MarketDepth` int DEFAULT NULL,
  `TransactTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `AccountSubjectType` int DEFAULT NULL,
  `SecurityID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `OrginData`  varbinary(4096) DEFAULT NULL,
  `JsonData` varchar(4096) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  INDEX `timems`(`timems`, `sn`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MarketDataXSwapDepth`
--

LOCK TABLES `MarketDataXSwapDepth` WRITE;
/*!40000 ALTER TABLE `MarketDataXSwapDepth` DISABLE KEYS */;
/*!40000 ALTER TABLE `MarketDataXSwapDepth` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `MarketDataBrokerBest`
--

DROP TABLE IF EXISTS `MarketDataBrokerBest`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `MarketDataBrokerBest` (
  `hjcode` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `sourceType` int DEFAULT NULL,
  `timems` bigint DEFAULT NULL,
  `recvms` bigint DEFAULT NULL,
  `sn` bigint DEFAULT NULL,
  `marketChan` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `BeginString` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgType` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgSeqNum` int DEFAULT NULL,
  `SendingTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MDBookType` int DEFAULT NULL,
  `MDSubBookType` int DEFAULT NULL,
  `SecurityID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `Symbol` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `TermToMaturity` varchar(255) COLLATE utf8mb4_bin DEFAULT NULL,
  `CFETSValuationYield` float(11,6) DEFAULT NULL,
  `Broker` varchar(255) COLLATE utf8mb4_bin DEFAULT NULL,
  `MarketID` varchar(255) COLLATE utf8mb4_bin DEFAULT NULL,
  `SecurityDesc` varchar(255) COLLATE utf8mb4_bin DEFAULT NULL,
  `LatestSubjectRating` varchar(255) COLLATE utf8mb4_bin DEFAULT NULL,
  `TransactTime` varchar(256) COLLATE utf8mb4_bin DEFAULT NULL,
  `OrginData`  varbinary(4096) DEFAULT NULL,
  `JsonData` varchar(4096) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  INDEX `timems`(`timems`, `sn`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MarketDataBrokerBest`
--

LOCK TABLES `MarketDataBrokerBest` WRITE;
/*!40000 ALTER TABLE `MarketDataBrokerBest` DISABLE KEYS */;
/*!40000 ALTER TABLE `MarketDataBrokerBest` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `MarketDataXbondBest`
--

DROP TABLE IF EXISTS `MarketDataXbondBest`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `MarketDataXbondBest` (
  `hjcode` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `sourceType` int DEFAULT NULL,
  `timems` bigint DEFAULT NULL,
  `recvms` bigint DEFAULT NULL,
  `sn` bigint DEFAULT NULL,
  `marketChan` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `BeginString` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgType` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MsgSeqNum` int DEFAULT NULL,
  `SendingTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MDReqID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MDBookType` int DEFAULT NULL,
  `SecurityID` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `MarketIndicator` int DEFAULT NULL,
  `TransactTime` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `OrginData`  varbinary(4096) DEFAULT NULL,
  `JsonData` varchar(4096) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  INDEX `timems`(`timems`, `sn`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MarketDataXbondBest`
--

LOCK TABLES `MarketDataXbondBest` WRITE;
/*!40000 ALTER TABLE `MarketDataXbondBest` DISABLE KEYS */;
/*!40000 ALTER TABLE `MarketDataXbondBest` ENABLE KEYS */;
UNLOCK TABLES;
SET FOREIGN_KEY_CHECKS = 1;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2023-09-18 13:27:29
