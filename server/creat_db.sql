-- -----------------------------------------------------
-- Table `Music`.`Artists`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `User`.`Password` ;

CREATE TABLE IF NOT EXISTS Password (
 UserId INT NOT NULL unique,
 Username VARCHAR(255) NOT NULL,
 Password VARCHAR(255) NOT NULL,
 PRIMARY KEY (Username));


-- -----------------------------------------------------
-- Table `Music`.`Genres`
-- -----------------------------------------------------
DROP TABLE IF EXISTS Files ;

CREATE TABLE IF NOT EXISTS Files (
 Filename VARCHAR(255) NOT NULL,
 Username VARCHAR(255) NOT NULL,
 PRIMARY KEY (Filename,Username));


