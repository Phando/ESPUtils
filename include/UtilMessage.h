/*
  UtilMessage.h - ESPUtils Library for development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.
*/

#if !defined(UTIL_MESSAGE_h)
#define UTIL_MESSAGE_h

#include <Arduino.h>
#include <sstream>
#include <iomanip>

using namespace std;

class UtilMessage;
typedef void (*UtilMessageCallback)(UtilMessage message);

//------------------------------------------------------------------------------------
class UtilMessage {
public:
    UtilMessage(){};
    UtilMessage(byte data){ write(data); };
    UtilMessage(vector<byte> data){ _data = data; };
    UtilMessage(string data){ write(data); };
    UtilMessage(byte command, string payload){ write(command); write(payload); };

    bool available();
    int bytesAvailable();

    int size();
    byte back();
    byte front();
    byte peek();
    byte read();
    unsigned char readChar();
    int readInt();
    string readColor();
    String readString();
    string readCString();
    vector<byte> readColorValue();

    void clear();
    void next();
    void reset();
    void write(byte data);
    void write(string data);
    void writeFront(byte data);
    void writeColor(string color);
    void writeColor(byte r, byte g, byte b);

private:
    int _dataIndex = 0;
    vector<byte> _data;
  
};

//------------------------------------------------------------------------------------

int UtilMessage::size(){
    return _data.size();
}

//------------------------------------------------------------------------------------

bool UtilMessage::available(){
    return _data.size() > 0;
}

//------------------------------------------------------------------------------------

int UtilMessage::bytesAvailable(){
    return _data.size() - _dataIndex;
}

//------------------------------------------------------------------------------------

void UtilMessage::clear(){ 
    _dataIndex = 0;
    _data.clear();
}

//------------------------------------------------------------------------------------

void UtilMessage::next(){ 
    _dataIndex++;
}

//------------------------------------------------------------------------------------

void UtilMessage::reset(){ 
    _dataIndex = 0;
}

//------------------------------------------------------------------------------------

byte UtilMessage::front(){
    return _data.front();
    //return _data.at(_dataIndex);
}

//------------------------------------------------------------------------------------

byte UtilMessage::back(){
    return *_data.end();
}

//------------------------------------------------------------------------------------

byte UtilMessage::peek(){
    if(_dataIndex < _data.size()){
        return _data.at(_dataIndex);
    }
    return '\0';
}

//------------------------------------------------------------------------------------

byte UtilMessage::read(){
    if(_dataIndex < _data.size()){
        return _data.at(_dataIndex++);
    }
    return '\0';
}

//------------------------------------------------------------------------------------

unsigned char UtilMessage::readChar(){
    return (unsigned char)read();
}

//------------------------------------------------------------------------------------

int UtilMessage::readInt(){
    return (int)read();
}

//------------------------------------------------------------------------------------

string UtilMessage::readCString(){
    return string(_data.begin()+_dataIndex, _data.end());
}

//------------------------------------------------------------------------------------

String UtilMessage::readString(){
    return readCString().c_str();
}

//------------------------------------------------------------------------------------

string UtilMessage::readColor(){
    if(bytesAvailable() < 3) return "";

    ostringstream str;
    str << hex << uppercase << setfill('0');
    
    for( int i = 0; i < 3; i++ ) {
        str << std::setw( 2 ) << readInt();
    }

    return str.str();
}

//------------------------------------------------------------------------------------

vector<byte> UtilMessage::readColorValue(){
    vector<byte> data(3); 
    
    // Length of 3 = R + G + B
    if(bytesAvailable()>2){    
        data.push_back(read()); // R
        data.push_back(read()); // G
        data.push_back(read()); // B
    }
    return data;
}

//------------------------------------------------------------------------------------

void UtilMessage::write(byte data){
    _data.push_back(data);
}

//------------------------------------------------------------------------------------

void UtilMessage::write(string data){
    _data.insert(_data.end(), data.begin(), data.end());
}

//------------------------------------------------------------------------------------

void UtilMessage::writeFront(byte data){
    _data.insert(_data.begin(),data);
}

//------------------------------------------------------------------------------------

void UtilMessage::writeColor(string color){
//     for (unsigned int i = 0; i < hex.length(); i += 2) {
//     std::string byteString = hex.substr(i, 2);
//     char byte = (char) strtol(byteString.c_str(), NULL, 16);
//     bytes.push_back(byte);
//   }
}
//------------------------------------------------------------------------------------ 

void UtilMessage::writeColor(byte r, byte g, byte b){
    write(r);
    write(g);
    write(b);
}

#endif