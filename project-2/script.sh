sshpass -p 'a' ssh jcharante@localhost -p8022 "zip -r project-2-backup-latest.zip Documents/project-2" && sshpass -p 'a' scp -P8022 jcharante@localhost:/home/jcharante/project-2-backup-latest.zip ./ && mv project-2-backup-latest.zip  project-2-backup-$(date +%m-%d-%H:%M:%S).zip

