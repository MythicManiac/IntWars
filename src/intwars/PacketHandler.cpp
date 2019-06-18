/*
IntWars playground server for League of Legends protocol testing
Copyright (C) 2012  Intline9 <Intline9@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Game.h"
#include "Packets.h"
#include "Logger.h"
#include <stdio.h>
//#undef min // No, do NOT do this.
//#define min(a, b)       ((a) < (b) ? (a) : (b))

void Game::initHandlers()
{
   memset(_handlerTable,0,sizeof(_handlerTable));
   registerHandler(&Game::handleKeyCheck,        PKT_KeyCheck, CHL_HANDSHAKE);
   registerHandler(&Game::handleLoadPing,        PKT_C2S_Ping_Load_Info, CHL_C2S);
   registerHandler(&Game::handleSpawn,           PKT_C2S_CharLoaded, CHL_C2S);
   registerHandler(&Game::handleMap,             PKT_C2S_ClientReady, CHL_LOADING_SCREEN);
   registerHandler(&Game::handleSynch,           PKT_C2S_SynchVersion, CHL_C2S);
   registerHandler(&Game::handleCastSpell,       PKT_C2S_CastSpell, CHL_C2S);
   //registerHandler(&Game::handleGameNumber,      PKT_C2S_GameNumberReq, CHL_C2S);
   registerHandler(&Game::handleQueryStatus,     PKT_C2S_QueryStatusReq, CHL_C2S);
   registerHandler(&Game::handleStartGame,       PKT_C2S_StartGame, CHL_C2S);
   registerHandler(&Game::handleNull,            PKT_C2S_Exit, CHL_C2S);
   registerHandler(&Game::handleView,            PKT_C2S_ViewReq, CHL_C2S);
   registerHandler(&Game::handleNull,            PKT_C2S_Click, CHL_C2S);
   //registerHandler(&Game::handleNull,            PKT_C2S_OpenShop, CHL_C2S);
   registerHandler(&Game::handleAttentionPing,   PKT_C2S_AttentionPing, CHL_C2S);
   registerHandler(&Game::handleChatBoxMessage , PKT_ChatBoxMessage, CHL_COMMUNICATION);
   registerHandler(&Game::handleMove,            PKT_C2S_MoveReq, CHL_C2S);
   registerHandler(&Game::handleNull,            PKT_C2S_MoveConfirm, CHL_C2S);
   registerHandler(&Game::handleSkillUp,		    PKT_C2S_SkillUp, CHL_C2S);
   registerHandler(&Game::handleEmotion,		    PKT_C2S_Emotion, CHL_C2S);
   registerHandler(&Game::handleBuyItem,		    PKT_C2S_BuyItemReq, CHL_C2S);
   registerHandler(&Game::handleSellItem,		    PKT_C2S_SellItem, CHL_C2S);
   registerHandler(&Game::handleSwapItems,		 PKT_C2S_SwapItems, CHL_C2S);
   registerHandler(&Game::handleNull,            PKT_C2S_LockCamera, CHL_C2S);
   registerHandler(&Game::handleNull,            PKT_C2S_StatsConfirm, CHL_C2S);
   registerHandler(&Game::handleClick,           PKT_C2S_Click, CHL_C2S);
   registerHandler(&Game::handleHeartBeat,       PKT_C2S_HeartBeat, CHL_GAMEPLAY);
}

void Game::registerHandler(bool (Game::*handler)(HANDLE_ARGS), PacketCmd pktcmd,Channel c)
{
	_handlerTable[pktcmd][c] = handler;
}

void Game::printPacket(const uint8 *buffer, uint32 size)
{
#define ppMIN(a, b)       ((a) < (b) ? (a) : (b))
	char* stringbuffer = new char[size+256]; // 256 bytes for the message around it.
	memset(stringbuffer, 0, (size + 256 )* sizeof(char));

   unsigned int i;
	sprintf(stringbuffer + strlen(stringbuffer), "Printing packet with size %u\n", size);
   
   for(i = 0; i < size; ++i) {
      if(i != 0&& i%16 == 0) {
         for(unsigned int j = i-16; j < i; ++j) {
         
            if(buffer[j] >= 32 && buffer[j] <= 126)
					sprintf(stringbuffer + strlen(stringbuffer), "%c", buffer[j]);
				else sprintf(stringbuffer + strlen(stringbuffer), ".");
         }
         
			sprintf(stringbuffer + strlen(stringbuffer), "");
      }
      
      if(i%16 == 0) {
			sprintf(stringbuffer + strlen(stringbuffer), "%04d-%04d ", i, ppMIN(i + 15, size - 1));
      }

		sprintf(stringbuffer + strlen(stringbuffer), "%02X ", buffer[i]);
   }
   
   for(i = ((16-i%16)%16); i > 0; --i)
		sprintf(stringbuffer + strlen(stringbuffer), "   ");
   
   for(i = size- (size%16 == 0 ? 16 : size%16); i < size; ++i) {
      if(buffer[i] >= 32 && buffer[i] <= 126)
			sprintf(stringbuffer + strlen(stringbuffer), "%c", buffer[i]);
      else
			sprintf(stringbuffer + strlen(stringbuffer), ".");
   }
   
	sprintf(stringbuffer + strlen(stringbuffer), "\n");
   for(i = 0; i < size; ++i) {
		sprintf(stringbuffer + strlen(stringbuffer), "\\x%02x", buffer[i]);
   }
	sprintf(stringbuffer + strlen(stringbuffer), "\n");

	CORE_INFO("%s", stringbuffer);
	delete[] stringbuffer;
}

void Game::printLine(uint8 *buf, uint32 len)
{
	//for(uint32 i = 0; i < len; i++)
		//PDEBUG_LOG(Logging,"%02X ", static_cast<uint8>(buf[i]));
	//PDEBUG_LOG(Logging,"\n");
}

bool Game::sendPacket(ENetPeer *peer, const uint8 *source, uint32 length, uint8 channelNo, uint32 flag)
{
	////PDEBUG_LOG_LINE(Logging," Sending packet:\n");
	//if(length < 300)
	//	printPacket(data, length);
   
   uint8* data = new uint8[length];
   memcpy(data, source, length);

	if(length >= 8)
		_blowfish->Encrypt(data, length-(length%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary

	ENetPacket *packet = enet_packet_create(data, length, flag);
	if(enet_peer_send(peer, channelNo, packet) < 0)
	{
      delete[] data;
		//PDEBUG_LOG_LINE(Logging,"Warning fail, send!");
		return false;
	}
   
   delete[] data;
	return true;
}

bool Game::sendPacket(ENetPeer *peer, const Packet& packet, uint8 channelNo, uint32 flag) {
   return sendPacket(peer, (const uint8*)&packet.getBuffer().getBytes()[0], packet.getBuffer().size(), channelNo, flag);
}

bool Game::broadcastPacket(uint8 *data, uint32 length, uint8 channelNo, uint32 flag)
{
	////PDEBUG_LOG_LINE(Logging," Broadcast packet:\n");
	//printPacket(data, length);

	if(length >= 8)
		_blowfish->Encrypt(data, length-(length%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary

	ENetPacket *packet = enet_packet_create(data, length, flag);

	enet_host_broadcast(_server, channelNo, packet);
	return true;
}

bool Game::broadcastPacket(const Packet& packet, uint8 channelNo, uint32 flag) {
   return broadcastPacket((uint8*)&packet.getBuffer().getBytes()[0], packet.getBuffer().size(), channelNo, flag);
}

bool Game::broadcastPacketTeam(uint8 team, const uint8 *data, uint32 length, uint8 channelNo, uint32 flag)
{
	for(ClientInfo* ci : players) {
      if(ci->getPeer() && ci->getTeam() == team) {
         sendPacket(ci->getPeer(), data, length, channelNo, flag);
      }
   }
   
	return true;
}

bool Game::broadcastPacketTeam(uint8 team, const Packet& packet, uint8 channelNo, uint32 flag)
{
	return broadcastPacketTeam(team, (const uint8*)&packet.getBuffer().getBytes()[0], packet.getBuffer().size(), channelNo, flag);
}

bool Game::broadcastPacketVision(Object* o, const Packet& packet, uint8 channelNo, uint32 flag) {
   return broadcastPacketVision(o, (const uint8*)&packet.getBuffer().getBytes()[0], packet.getBuffer().size(), channelNo, flag);
}

bool Game::broadcastPacketVision(Object* o, const uint8 *data, uint32 length, uint8 channelNo, uint32 flag) {
   
   for(int i = 0; i < 2; ++i) {
      if(o->isVisibleByTeam(i)) {
         broadcastPacketTeam((i == 0) ? TEAM_BLUE : TEAM_PURPLE, data, length, channelNo, flag);
      }
   }
	return true;
}

bool Game::handlePacket(ENetPeer *peer, ENetPacket *packet, uint8 channelID)
{
	if(packet->dataLength >= 8)
	{
		if(peerInfo(peer))
			_blowfish->Decrypt(packet->data, packet->dataLength-(packet->dataLength%8)); //Encrypt everything minus the last bytes that overflow the 8 byte boundary
	}

	PacketHeader *header = reinterpret_cast<PacketHeader*>(packet->data);	
	bool (Game::*handler)(HANDLE_ARGS) = _handlerTable[header->cmd][channelID];
	
	if(handler)
	{
		return (*this.*handler)(peer,packet);
	}
	else
	{
		CORE_WARNING("Unhandled OpCode %02X", header->cmd);
		printPacket(packet->data, packet->dataLength);
	}
	return false;	
}