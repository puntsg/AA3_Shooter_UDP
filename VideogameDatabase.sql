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
    INSERT INTO Players (Username, Password, Score) 
    VALUES (iUsername, iPassword, 0);
END//
DELIMITER ;

-- Volcando estructura para procedimiento videogame.DeletePlayer
DELIMITER //
CREATE PROCEDURE `DeletePlayer`(IN iId INT UNSIGNED)
BEGIN
    DELETE FROM Players WHERE Id = iId;
END//
DELIMITER ;

-- Volcando estructura para procedimiento videogame.GetRanking
DELIMITER //
CREATE PROCEDURE `GetRanking`(IN iUsername VARCHAR(50))
BEGIN
    (SELECT Id, Username, Score 
     FROM Players 
     ORDER BY Score DESC 
     LIMIT 10)
    UNION
    (SELECT Id, Username, Score 
     FROM Players 
     WHERE Username = iUsername)
    ORDER BY Score DESC;
END//
DELIMITER ;

-- Volcando estructura para procedimiento videogame.LoginPlayer
DELIMITER //
CREATE PROCEDURE `LoginPlayer`(IN iUsername VARCHAR(50), IN iPassword CHAR(64))
BEGIN
    SELECT Id, Username, Score 
    FROM Players 
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
	(1, 'a', 'ca978112ca1bbdcafac2', 0),
	(2, 'b', '3e23e8160039594a3389', 0),
	(3, 'c', '2e7d2c03a9507ae265ec', 0),
	(4, 'edgar', '8849853b957fe153b705', 0),
	(5, 'sergi', '4d0af2adc4ed954308d9', 0),
	(6, 'carol', '4c26d9074c27d89ede59', 0),
	(7, 'Richard', 'b10883f7f6046846019e', 0),
	(8, 'Radev', '932cc859d4215f975c3c', 0),
	(9, 'Alex', 'f1aad41d0dd24ed8a193', 0);

-- Volcando estructura para procedimiento videogame.UpdateScore
DELIMITER //
CREATE PROCEDURE `UpdateScore`(IN iUsername VARCHAR(50), IN iAddedPoints INT)
BEGIN
    UPDATE Players 
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
