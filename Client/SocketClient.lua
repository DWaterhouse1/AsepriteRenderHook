local ws
local dlg = Dialog()
local spr = app.activeSprite

-- image buffers are necessary to preserve extent data
-- the cels may not be the same size as the sprite proper
local albdBuf = Image(spr.width, spr.height, ColorMode.RGB)
local normBuf = Image(spr.width, spr.height, ColorMode.RGB)

local sendImage
local sendInit
local onSiteChange

local function finish()
  if ws ~= nil then ws:close() end
  if dlg ~= nil then dlg:close() end
  spr.events:off(sendImage)
  app.events:off(onSiteChange)
  dlg = nil
  spr = nil
end

sendInit = function()
	if albdBuf.width ~= spr.width or albdBuf.height	~= spr.height
	then
		albdBuf:resize(spr.width, spr.height)
	end

	if normBuf.width ~= spr.width or normBuf.height	~= spr.height
	then
		normBuf:resize(spr.width, spr.height)
	end
	
	for _,layer in ipairs(spr.layers) do
		if layer.name == "Normal"
		then
			normBuf:clear()
			normBuf:drawImage(layer.cels[1].image, layer.cels[1].position)
		end

		if layer.name == "Albedo"
		then
			albdBuf:clear()
			albdBuf:drawImage(layer.cels[1].image, layer.cels[1].position)
		end
	end

	ws:sendBinary(
		string.pack("<LLL", string.byte("I"), albdBuf.width, albdBuf.height),
		albdBuf.bytes,
		normBuf.bytes)
end

sendImage = function()
	if albdBuf.width ~= spr.width or albdBuf.height	~= spr.height
	then
		albdBuf:resize(spr.width, spr.height)
	end

	if normBuf.width ~= spr.width or normBuf.height	~= spr.height
	then
		normBuf:resize(spr.width, spr.height)
	end
	
	for _,layer in ipairs(spr.layers) do
		if layer.name == "Normal"
		then
			normBuf:clear()
			normBuf:drawImage(layer.cels[1].image, layer.cels[1].position)
		end

		if layer.name == "Albedo"
		then
			albdBuf:clear()
			albdBuf:drawImage(layer.cels[1].image, layer.cels[1].position)
		end
	end

	ws:sendBinary(
		string.pack("<LLL", string.byte("R"), albdBuf.width, albdBuf.height),
		albdBuf.bytes,
		normBuf.bytes)
end

local frame = -1
onSiteChange = function()
	if app.activeSprite ~= spr
	then
		for _, s in ipairs(app.sprites)
		do
			if s == spr then
				break
			end
		end
		
		finish()
	else
		if app.activeFrame.frameNumber ~= frame
		then
			frame = app.activeFrame.frameNumber
			sendImage()
		end
	end
end

local function receive(t, message)
  if t == WebSocketMessageType.OPEN
  then
    dlg:modify{id="status", text="Sync ON"}
		sendInit()
  elseif t == WebSocketMessageType.CLOSE and dlg ~= nil
	then
		dlg:modify{id="status", text="No connection"}
		spr.events:off(sendImage)
		app.events:off(onSiteChange)
	elseif t == WebSocketMessageType.TEXT
	then
		if message == "READY" or message == "WAKE"
		then
			spr.events:on('change', sendImage)
			app.events:on('sitechange', onSiteChange)
		elseif message == "SLEEP"
		then
			spr.events:off(sendImage)
			app.events:off(onSiteChange)
		end
  end
end

ws = WebSocket{ url="ws://localhost:30001", onreceive=receive, deflate=false}

dlg:label{id="status", text="Connecting..."}
dlg:button{text="Cancel", onclick=finish}

ws:connect()
dlg:show{ wait = false }