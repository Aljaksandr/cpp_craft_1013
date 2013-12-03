#include <conio.h>

#include <fstream>
#include <iostream>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include "market_data_receiver.h"
#include "../multicast_communication/market_messages_pump.h"
#include "../multicast_communication/market_data_processor.h"

namespace multicast_communication
{
  void io_thread(boost::asio::io_service& service);

  void save_results(market_data_processor_ptr& mdp);
}


int main()
{
  using namespace multicast_communication;

  static const char* kConfigFilename = "config.ini";

  std::ifstream config_ifs(kConfigFilename);
  if (!config_ifs.good())
  {
    std::cerr << "Can't open configuration file: " 
	      << kConfigFilename << std::endl; 
    return -1;
  }

  size_t trade_thread_size = 0;
  size_t quote_thread_size = 0;

  size_t quote_connections_count = 0;
  size_t trade_connections_count = 0;
  std::vector<connection_point> quote_connections;
  std::vector<connection_point> trade_connections;

  // TO DO: move this ugliness (read settings) into separate class

  // read configuration, without any convenient tools
  std::string line;

  try {
    getline(config_ifs, line);
    trade_thread_size = boost::lexical_cast<size_t>(line.c_str());
    getline(config_ifs, line);
    quote_thread_size = boost::lexical_cast<size_t>(line.c_str());
    getline(config_ifs, line);
    trade_connections_count = boost::lexical_cast<size_t>(line.c_str());
  } catch (const boost::bad_lexical_cast& e) {
    std::cerr << "Error occured, when configuration file had been read. "
      << e.what() << std::endl;
    return -1;
  }

  for (size_t i = 0; i < trade_connections_count; ++i)
  {
    getline(config_ifs, line);
    std::string address, port;
    size_t pos = line.find(' ');
    if (pos == std::string::npos)
      pos = line.find('\t');
    if (pos != std::string::npos)
    {
      address = line.substr(0, pos);
      while (pos < line.size() 
        && (line[pos] == ' ' || line[pos] == '\t')) 
      {
        ++pos;
      }
      unsigned short p = 0;
      if (pos < line.size())
        port = line.substr(pos);
      try {
        p = boost::lexical_cast<unsigned short>(port.c_str());
      } catch (boost::bad_lexical_cast& e) {
        std::cerr << "Error occured, when configuration file had been read. "
          << e.what() << std::endl;
        return -1;
      }

      trade_connections.push_back(connection_point(address, p));      
    } // if (pos != std::string::npos)
  } // for (size_t i = 0; i < trade_connections_count; ++i)


  try {
    getline(config_ifs, line);
    quote_connections_count = boost::lexical_cast<size_t>(line.c_str());
  } catch (const boost::bad_lexical_cast& e) {
    std::cerr << "Error occured, when configuration file had been read. "
      << e.what() << std::endl;
    return -1;
  }

  for (size_t i = 0; i < quote_connections_count; ++i)
  {
    std::string line;
    getline(config_ifs, line);
    std::string address, port;
    size_t pos = line.find(' ');
    if (pos == std::string::npos)
      pos = line.find('\t');
    if (pos != std::string::npos)
    {
      address = line.substr(0, pos);
      while (pos < line.size() && (line[pos] == ' '
        || line[pos] == '\t'))
      { 
        ++pos;
      }

      unsigned short p = 0;
      if (pos < line.size())
        port = line.substr(pos);
      try {
        p = boost::lexical_cast<unsigned short>(port.c_str());
      } catch (boost::bad_lexical_cast& e) {
        std::cerr << "Error occured, when configuration file had been read. "
          << e.what() << std::endl;
        return -1;
      }

      quote_connections.push_back(connection_point(address, p));      
    } // if (pos != std::string::npos)
  } // for (size_t i = 0; i < quote_thread_size; ++i)

  boost::asio::io_service trade_io_service;  
  boost::asio::io_service quote_io_service;  
  market_data_processor_ptr market_data_processor(new market_data_processor_impl());
  market_data_receiver_ptr market_data_receiver(new market_data_receiver(market_data_processor));

  message_receiver_delegate_ptr trade_msg_recvr_delegate(
      new tmessage_receiver_delegate(market_data_receiver));
  message_receiver_delegate_ptr quote_msg_recvr_delegate(
      new qmessage_receiver_delegate(market_data_receiver));

  boost::thread_group io_workers;
  
  std::vector<market_messages_pump_ptr> trade_messages_pumps;
  std::vector<market_messages_pump_ptr> quote_messages_pumps;

  for (size_t i = 0; i < trade_connections.size(); ++i)
  {
    const connection_point& trade_cp = trade_connections[i];
    market_messages_pump_ptr tmessage_pump_ptr(new market_messages_pump(
                              trade_io_service, trade_msg_recvr_delegate));
    trade_messages_pumps.push_back(tmessage_pump_ptr);
    tmessage_pump_ptr->start(trade_cp.first, trade_cp.second);
  }

  for (size_t i = 0; i < quote_connections.size(); ++i)
  {
    const connection_point& quote_cp = quote_connections[i];
    market_messages_pump_ptr qmessage_pump_ptr(new market_messages_pump(
                              quote_io_service, quote_msg_recvr_delegate));
    quote_messages_pumps.push_back(qmessage_pump_ptr);
    qmessage_pump_ptr->start(quote_cp.first, quote_cp.second);
  }


  for (size_t i = 0; i < trade_thread_size; ++i)
    io_workers.create_thread(boost::bind(io_thread, boost::ref(trade_io_service)));

  for (size_t i = 0; i < quote_thread_size; ++i)
    io_workers.create_thread(boost::bind(io_thread, boost::ref(quote_io_service)));

  int key = 0x00;
  std::cout << "Press [Ctrl] + [C] to stop\n";
  while (key != 0x03)
    key = _getch();

  for (size_t i = 0; i < trade_messages_pumps.size(); ++i)
    trade_messages_pumps[i]->stop();

  for (size_t i = 0; i < quote_messages_pumps.size(); ++i)
    quote_messages_pumps[i]->stop();

  trade_io_service.stop();
  quote_io_service.stop();
  io_workers.join_all();

  save_results(market_data_processor);
  
  return 0;
}

void multicast_communication::io_thread(boost::asio::io_service& service)
{
  service.run();
}

void multicast_communication::save_results(market_data_processor_ptr& mdp)
{
  static const char* kOutFilename = "market_data.dat";
  std::ofstream ofs(kOutFilename);

  if (!ofs.good())
  {
    std::cerr << "Can't open file: " << kOutFilename << std::endl;
    return;
  }

  mdp->dump_messages(ofs);
}
