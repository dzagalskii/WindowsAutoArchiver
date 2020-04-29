# WindowsAutoArchiver
A simple Windows service for automatically backing up files.
The program works as a Windows service (s). The program backs up the data from the specified directory.
## Functions:
- The program is not interactive, all settings are set using the configuration file;
- The program creates an archive with a backup of data from the specified directory;
- Files included in the backup are specified by mask (using *,? characters) or directly by names;
- Backup is created as a ZIP archive;
- If existing files change or new files appear in the source directory (corresponding to the specified mask), the archive with backup is automatically supplemented with new data (or existing ones are replaced);
- Installation, uninstall, start, stop from command line (implementation of its SCP);
- Properly stop and restart the service on request from SCM (processing commands sent through the Services snap-in);
- File names and directories are specified in UNC format.
### The configuration file must contain at least:
- Name of the directory from which the backup is performed;
- Name of archive file and directory in which it should be located;
- Masks/names of files to be backed up.
