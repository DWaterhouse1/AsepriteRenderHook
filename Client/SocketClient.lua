local ws
local dlg = Dialog()
local spr = app.activeSprite

-- image buffers are necessary to preserve extent data
-- the cels may not be the same size as the sprite proper
local buf = Image(spr.width, spr.height, ColorMode.RGB)

local sendImage
local onSiteChange

local function finish()
  if ws ~= nil then ws:close() end
  if dlg ~= nil then dlg:close() end
  spr.events:off(sendImage)
  app.events:off(onSiteChange)
  dlg = nil
  spr = nil
end

sendImage = function()
	if buf.width ~= spr.width or buf.height	~= spr.height then
		buf:resize(spr.width, spr.height)
	end
	
	--buf:clear()
	--buf:drawSprite(spr, app.activeFrame.frameNumber)
	--ws:sendBinary(string.pack("<LLL", string.byte("I"), buf.width, buf.height), buf.bytes)
	
	for _,layer in ipairs(spr.layers) do
		if layer.name == "Normal" then
			buf:clear()
			buf:drawImage(layer.cels[1].image, layer.cels[1].position)
			ws:sendBinary(string.pack("<LLL", string.byte("N"), buf.width, buf.height), buf.bytes)
		elseif layer.name == "Diffuse" then
			buf:clear()
			buf:drawImage(layer.cels[1].image, layer.cels[1].position)
			ws:sendBinary(string.pack("<LLL", string.byte("D"), buf.width, buf.height), buf.bytes)
		end
	end
end

local frame = -1
onSiteChange = function()
	if app.activeSprite ~= spr then
		for _, s in ipairs(app.sprites) do
			if s == spr then
				break
			end
		end
		
		finish()
	else
		if app.activeFrame.frameNumber ~= frame then
			frame = app.activeFrame.frameNumber
			sendImage()
		end
	end
end

local function receive(t, message)
    if t == WebSocketMessageType.OPEN then
        dlg:modify{id="status", text="Sync ON"}
        spr.events:on('change', sendImage)
        app.events:on('sitechange', onSiteChange)
        sendImage()

    elseif t == WebSocketMessageType.CLOSE and dlg ~= nil then
        dlg:modify{id="status", text="No connection"}
        spr.events:off(sendImage)
        app.events:off(onSiteChange)
    end
end

ws = WebSocket{ url="ws://localhost:30001", onreceive=receive, deflate=false}

dlg:label{id="status", text="Connecting..."}
dlg:button{text="Cancel", onclick=finish}

ws:connect()
dlg:show{ wait = false }