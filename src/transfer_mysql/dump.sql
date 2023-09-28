/*
 Navicat Premium Data Transfer

 Source Server         : 172.24.13.178
 Source Server Type    : MySQL
 Source Server Version : 80023
 Source Host           : 172.24.13.178:3306
 Source Schema         : yctest

 Target Server Type    : MySQL
 Target Server Version : 80023
 File Encoding         : 65001

 Date: 22/09/2023 13:52:06
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for c_1000
-- ----------------------------
DROP TABLE IF EXISTS `c_1000`;
CREATE TABLE `c_1000`  (
  `key` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `value` varbinary(4096) NULL DEFAULT NULL,
  `timems` bigint(0) NULL DEFAULT NULL,
  `recvms` bigint(0) NULL DEFAULT NULL,
  `sn` bigint(0) NULL DEFAULT NULL,
  `pub_user` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NULL DEFAULT NULL,
  `topic` int(0) NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
