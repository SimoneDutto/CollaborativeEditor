-- -----------------------------------------------------
-- Table `Music`.`Artists`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `User`.`Password` ;

CREATE TABLE IF NOT EXISTS Password (
 Username VARCHAR(255) NOT NULL,
 Password VARCHAR(255) NOT NULL,
 PRIMARY KEY (Username));


-- -----------------------------------------------------
-- Table `Music`.`Genres`
-- -----------------------------------------------------
DROP TABLE IF EXISTS Files ;

CREATE TABLE IF NOT EXISTS Files (
 FileName VARCHAR(255) NOT NULL,
 FileId INT NOT NULL,
 UserId INT NOT NULL,
 PRIMARY KEY (FileName,UserId));


INSERT INTO Files VALUES("text.txt",1,1);
INSERT INTO Files VALUES("text.txt",1,2);


