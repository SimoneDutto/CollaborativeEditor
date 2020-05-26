-- -----------------------------------------------------
-- Table `Music`.`Artists`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `User`.`Password` ;

CREATE TABLE Password (
 Username VARCHAR(255) NOT NULL,
 Password VARCHAR(255) NOT NULL,
 PRIMARY KEY (Username));
-- -----------------------------------------------------
-- Table `Music`.`Genres`
-- -----------------------------------------------------
DROP TABLE IF EXISTS Files ;

CREATE TABLE Files( 
 FileName VARCHAR(255) NOT NULL, 
 FileId INT NOT NULL, 
 UserId INT NOT NULL, 
 SiteCounter INT NOT NULL, 
 PRIMARY KEY (FileName,UserId));


