# Systemd service for CIB Quasar

## Installation instructions

1. Edit the template to reflect your installation
2. Copy the file to `/lib/systemd/system` directory
3. Enable the service:
```bash
systemctl enable cib_quasar
```
4. Start the service
```bash
systemctl start cib_quasar
```