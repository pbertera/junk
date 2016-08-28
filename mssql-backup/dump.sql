----------------------------------------------------------------------------------------------------
-- Example: 
-- Clar stored procedure:
-- osql -E -Q "DROP PROC Full_Backup_user_db"
-- Import stored procedure:
-- osql -E -i dump.sql
-- osql -E -Q "EXEC Full_Backup_user_db @Path = 'D:\Backup-db'"
----------------------------------------------------------------------------------------------------
CREATE PROC Full_Backup_user_db
@Path VARCHAR(100)
--@HistoryDays int --number of days to hold the database
AS

SET NOCOUNT ON

DECLARE @Now CHAR(14) -- current date in the form of yyyymmddhhmmss
DECLARE @DBName sysname -- stores the database name that is currently being processed
DECLARE @SQL VARCHAR(7000) -- stores the dynamically created xp_backup_database command
DECLARE @cmd sysname -- stores the dynamically created DOS command
DECLARE @Result INT -- stores the result of the dir DOS command
DECLARE @RowCnt INT -- stores @@ROWCOUNT

--make sure our path has a trailing slash otherwise we will have a mess of prefixed directories
if SUBSTRING(@Path, len(@Path), 1) != '\'
Set @Path = @Path + '\'

-- Get the list of the databases to be backed up, does not include master, model, msdb, tempdb, Northwind, or pubs
SELECT name
INTO #WhichDatabase
FROM master.dbo.sysdatabases
WHERE name NOT IN ('master', 'model', 'msdb', 'pubs', 'tempdb', 'Northwind')
ORDER BY name

-- Get the database to be backed up
SELECT TOP 1 @DBName = name
FROM #WhichDatabase

SET @RowCnt = @@ROWCOUNT

-- Iterate throught the temp table until no more databases need to be backed up
WHILE @RowCnt <> 0
BEGIN

-- Build the xp_backup_database command dynamically
SELECT @SQL = ''
SELECT @SQL = @SQL + 'BACKUP DATABASE [' + @DBName + ']' + CHAR(10)
SELECT @SQL = @SQL + 'TO DISK = ''' + @Path + @DBName + '.BAK''' + CHAR(10)
SELECT @SQL = @SQL + 'WITH INIT ' + CHAR(10)
print @SQL
-- Backup the database using xp_backup_database
EXEC (@SQL)

-- To move onto the next database, the current database name needs to be deleted from the temp table
DELETE
FROM #WhichDatabase
WHERE name = @DBName

-- Get the database to be backed up
SELECT TOP 1 @DBName = name
FROM #WhichDatabase

SET @RowCnt = @@ROWCOUNT

-- Let the system rest for 5 seconds before starting on the next backup
WAITFOR DELAY '00:00:05'

END

DROP TABLE #WhichDatabase

SET NOCOUNT OFF

RETURN

GO