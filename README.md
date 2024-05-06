#Simplified OpenStack Object Storage   
1. download <user/object> - requires a group or user and object name and will download to Downloads folder and show what disk it was stored on. If file does not exist,it will tell you so.
2. list <user> - needs a user or group name and will return ls -lrt in all directories for that user.
3. upload <user/object> - file needs to be in current directory. Will upload to system and create backup.
4. delete <user/object> - confirms if deleted or not
5. add <disk> - add disk based on address
6. remove <disk> - rm disk based on address
7. clean - cleans disks.

When Sever is Killed all partiotn information is lost, so you will have to reupload your data.

