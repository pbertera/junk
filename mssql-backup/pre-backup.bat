set dumpdir=D:\backup-db\
set procfile=D:\MSSQL\dump.sql

REM Dump the default instance

osql -E -Q "DROP PROC Full_Backup_user_db"
osql -E -i %procfile%
osql -E -Q "EXEC Full_Backup_user_db @Path = '%dumpdir%'"

REM Dump the named instance "NAMED_INSTANCE" 

osql -S MY-SQLSERVER\NAMED_INSTANCE -E -Q "DROP PROC Full_Backup_user_db"
osql -S MY-SQLSERVER\NAMED_INSTANCE -E -i %procfile%
osql -S MY-SQLSERVER\NAMED_INSTANCE -E -Q "EXEC Full_Backup_user_db @Path = '%dumpdir%'"
