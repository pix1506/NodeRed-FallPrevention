#!/bin/bash

# 更新系統
sudo apt update
sudo apt upgrade -y

# 安裝 curl (用於下載 nvm)
sudo apt install -y curl

# 安裝 nvm (Node Version Manager)
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.0/install.sh | bash

# 重新加載 .bashrc 以激活 nvm
export NVM_DIR="$([ -z "${XDG_CONFIG_HOME-}" ] && printf %s "${HOME}/.nvm" || printf %s "${XDG_CONFIG_HOME}/nvm")"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"  # This loads nvm

# 安裝 Node.js 20 版本
nvm install 20

# 檢查 Node.js 和 npm 版本
node -v
npm -v

# 安裝 Node-RED
npm install -g --unsafe-perm node-red

# 啟動 Node-RED
node-red &

# 等待 Node-RED 啟動
sleep 10

# 自動打開瀏覽器並連接到 localhost:1880
xdg-open http://localhost:1880 &

####-----

# 取得腳本所在目錄
SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
FLOW_FILE="$SCRIPT_DIR/flows.json"

# 輸出 SCRIPT_DIR 的值，進行檢查
echo "Script directory: $SCRIPT_DIR"

# 檢查 flows.json 是否存在
if [ ! -f "$FLOW_FILE" ]; then
    echo "flows.json file not found in $SCRIPT_DIR. Please make sure it exists."
    exit 1
fi

# 將 flows.json 複製到 Node-RED 目錄
echo "Importing flows.json to Node-RED directory..."
cp "$FLOW_FILE" "$HOME/.node-red/flows_$(hostname).json"

# 安裝 Node-RED 所需的節點模組
echo "Installing required Node-RED modules..."
cd "$HOME/.node-red"

# 如果 package.json 不存在，則初始化一個
if [ ! -f "package.json" ]; then
    echo "No package.json found, creating one..."
    npm init -y --scope=node-red-project
    # 修改 package.json 名稱，避免使用 ".node-red" 作為名稱
    sed -i 's/"name": "node-red-project"/"name": "my-node-red-project"/' package.json
fi

# 安裝 Node-RED Dashboard UI 模組
npm install node-red-dashboard

# 安裝 node-red-contrib-aedes 模組
npm install node-red-contrib-aedes

# 安裝其他模組
npm install

# 創建 systemd 服務以便 Node-RED 在系統啟動時自動啟動
echo "Setting up Node-RED as a systemd service..."
sudo bash -c 'cat <<EOF > /etc/systemd/system/nodered.service
[Unit]
Description=Node-RED
After=network.target

[Service]
ExecStart=/usr/bin/env node-red --max-old-space-size=512
WorkingDirectory=/home/$USER/.node-red
User=$USER
Group=$USER
Nice=10
SyslogIdentifier=Node-RED
StandardOutput=syslog
Restart=on-failure
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
EOF'

# 重新加載 systemd 並啟動 Node-RED
echo "Reloading systemd and starting Node-RED..."
sudo systemctl daemon-reload
sudo systemctl enable nodered
sudo systemctl start nodered

# 檢查 Node-RED 狀態
echo "Checking Node-RED status..."
sudo systemctl status nodered

# 自動開啟瀏覽器連接到 Node-RED
echo "Opening Node-RED in the default web browser..."
xdg-open http://localhost:1880 &

echo "Node-RED installation and setup complete. You can access it at http://localhost:1880"

