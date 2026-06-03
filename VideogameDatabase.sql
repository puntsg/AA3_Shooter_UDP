-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Versión del servidor:         10.4.32-MariaDB - mariadb.org binary distribution
-- SO del servidor:              Win64
-- HeidiSQL Versión:             12.15.0.7171
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


-- Volcando estructura de base de datos para videogame
CREATE DATABASE IF NOT EXISTS `videogame` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci */;
USE `videogame`;

-- Volcando estructura para procedimiento videogame.AddPlayer
DELIMITER //
CREATE PROCEDURE `AddPlayer`(IN iUsername VARCHAR(50), IN iPassword CHAR(64))
BEGIN
    INSERT INTO players (Username, Password, Score) 
    VALUES (iUsername, iPassword, 0);
END//
DELIMITER ;

-- Volcando estructura para procedimiento videogame.DeletePlayer
DELIMITER //
CREATE PROCEDURE `DeletePlayer`(IN iId INT UNSIGNED)
BEGIN
    DELETE FROM players WHERE Id = iId;
END//
DELIMITER ;

-- Volcando estructura para procedimiento videogame.GetRanking
DELIMITER //
CREATE PROCEDURE `GetRanking`(IN iUsername VARCHAR(50))
BEGIN
    SELECT Id, Username, Score
    FROM (
        SELECT 0 AS SortGroup, topPlayers.Id, topPlayers.Username, topPlayers.Score
        FROM (
            SELECT Id, Username, Score
            FROM players
            ORDER BY Score DESC, Username ASC
            LIMIT 10
        ) AS topPlayers
        UNION ALL
        SELECT 1 AS SortGroup, p.Id, p.Username, p.Score
        FROM players AS p
        WHERE p.Username = iUsername
          AND NOT EXISTS (
              SELECT 1
              FROM (
                  SELECT Username
                  FROM players
                  ORDER BY Score DESC, Username ASC
                  LIMIT 10
              ) AS topNames
              WHERE topNames.Username = p.Username
          )
    ) AS rankingRows
    ORDER BY SortGroup ASC, Score DESC, Username ASC;
END//
DELIMITER ;

-- Volcando estructura para procedimiento videogame.LoginPlayer
DELIMITER //
CREATE PROCEDURE `LoginPlayer`(IN iUsername VARCHAR(50), IN iPassword CHAR(64))
BEGIN
    SELECT Id, Username, Score 
    FROM players 
    WHERE Username = iUsername AND Password = iPassword;
END//
DELIMITER ;

-- Volcando estructura para tabla videogame.players
CREATE TABLE IF NOT EXISTS `players` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Username` varchar(50) DEFAULT NULL,
  `Password` char(64) DEFAULT NULL,
  `Score` int(10) unsigned DEFAULT 0,
  PRIMARY KEY (`Id`),
  UNIQUE KEY `Username` (`Username`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- Volcando datos para la tabla videogame.players: ~9 rows (aproximadamente)
DELETE FROM `players`;
INSERT INTO `players` (`Id`, `Username`, `Password`, `Score`) VALUES
	(1, 'a', 'ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb', 0),
	(2, 'b', '3e23e8160039594a33894f6564e1b1348bbd7a0088d42c4acb73eeaed59c009d', 0),
	(3, 'c', '2e7d2c03a9507ae265ecf5b5356885a53393a2029d241394997265a1a25aefc6', 0),
	(4, 'edgar', '8849853b957fe153b7056d0e7d65f99fb21070daf5122ddf1d7c942d4643c33d', 0),
	(5, 'sergi', '4d0af2adc4ed954308d9af5dc082058266e72db750dfef7728d5e830a9a31bf7', 0),
	(6, 'carol', '4c26d9074c27d89ede59270c0ac14b71e071b15239519f75474b2f3ba63481f5', 0),
	(7, 'Richard', '1deb0a3b86750d0e4a4c21dc9601736954ae5ccac654c57f3d45563a1f595998', 0),
	(8, 'Radev', '835ed5ddd08a2ea98a45338e7d2f76b5500ef27787131bb8fd9d6b3672ac0b5a', 0),
	(9, 'Alex', 'db74c940d447e877d119df613edd2700c4a84cd1cf08beb7cbc319bcfaeab97a', 0);

-- Volcando estructura para procedimiento videogame.UpdateScore
DELIMITER //
CREATE PROCEDURE `UpdateScore`(IN iUsername VARCHAR(50), IN iAddedPoints INT)
BEGIN
    UPDATE players 
    SET Score = CASE 
        WHEN iAddedPoints < 0 AND Score < ABS(iAddedPoints) THEN 0 
        ELSE Score + iAddedPoints 
    END 
    WHERE Username = iUsername;
END//
DELIMITER ;

/*!40103 SET TIME_ZONE=IFNULL(@OLD_TIME_ZONE, 'system') */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
