///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2006 Alexander Wemmer 
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
///////////////////////////////////////////////////////////////////////////////

/*! \file    transport_layer_iface.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QByteArray>
#include <QDataStream>

#include "transport_layer_iface.h"

/////////////////////////////////////////////////////////////////////////////

QString TransportLayerIface::getSocketErrorText(QAbstractSocket::SocketError error)
{
    QString error_msg;
    
    switch(error)
    {
        case(QAbstractSocket::ConnectionRefusedError):
            error_msg = "Connection refused"; 
            break;
        case(QAbstractSocket::RemoteHostClosedError):
            error_msg = "Connection closed by remote host"; 
            break;
        case(QAbstractSocket::HostNotFoundError):
            error_msg = "Host not found"; 
            break;
        case(QAbstractSocket::SocketAccessError):
            error_msg = "Socket access error"; 
            break;
        case(QAbstractSocket::SocketResourceError):
            error_msg = "Socket resource error"; 
            break;
        case(QAbstractSocket::SocketTimeoutError):
            error_msg = "Socket timout"; 
            break;
        case(QAbstractSocket::DatagramTooLargeError):
            error_msg = "Datagram too large"; 
            break;
        case(QAbstractSocket::NetworkError):
            error_msg = "Network error"; 
            break;
        case(QAbstractSocket::AddressInUseError):
            error_msg = "Address in use"; 
            break;
        case(QAbstractSocket::SocketAddressNotAvailableError):
            error_msg = "Socket address not available"; 
            break;
        case(QAbstractSocket::UnsupportedSocketOperationError):
            error_msg = "Unsupported socket operation"; 
            break;
        default:
        case(QAbstractSocket::UnknownSocketError):
            error_msg = "Unkown error"; 
            break;
    }

    return error_msg;
}

/////////////////////////////////////////////////////////////////////////////

void TransportLayerIface::enframeData(qint16 data_type, const QByteArray& data, QByteArray& enframed_data)
{
    enframed_data.clear();
    QDataStream write_stream(&enframed_data, QIODevice::WriteOnly);

    qint32 overall_length = data.count() + sizeof(qint32)*2 + sizeof(qint16)*2;
    qint16 checksum = qChecksum(data.data(), data.count());

    write_stream << (qint32) overall_length
                 << data_type 
                 << (qint32) data.count();

    write_stream.writeRawData(data.data(), data.count());
    write_stream << (qint16) checksum;

    MYASSERT(enframed_data.count() == overall_length);

//     qDebug("TransportLayerIface:enframeData: "
//            "framed overall data (%d) , real data (%d), checksum (%d)",
//            overall_length, data.count(), checksum);
}

/////////////////////////////////////////////////////////////////////////////

bool TransportLayerIface::deframeReceivedData(QByteArray& data,
                                              qint16& data_type,
                                              QByteArray& deframed_data)
{
    deframed_data.clear();
    if (data.count() < (int)sizeof(qint32)) 
    {
//         qDebug("TransportLayerIface:deframeReceivedData: "
//                "data count (%d) too small to read overall length",
//                data.count());
        return false;
    }

    QDataStream read_stream(data);

    qint32 overall_length;
    read_stream >> overall_length;

    if (data.count() < overall_length) 
    {
//         qDebug("TransportLayerIface:deframeReceivedData: "
//                "data count (%d) < awaited overall length (%d)",
//                data.count(), overall_length);
        return false;
    }

//     qDebug("TransportLayerIface:deframeReceivedData: got %d bytes, awaited %d", 
//            data.count(), overall_length);
    
    qint32 data_length;
    read_stream >> data_type >> data_length;

    deframed_data.resize(data_length);
    int read_bytes = read_stream.readRawData(deframed_data.data(), data_length);
    MYASSERT(read_bytes == data_length);
    MYASSERT(deframed_data.count() == data_length);
    qint16 our_checksum = qChecksum(deframed_data.data(), deframed_data.count());

    qint16 received_checksum;
    read_stream >> received_checksum;

    data.remove(0, overall_length);

    if (our_checksum  != received_checksum)
    {
        qCritical("TransportLayerIface:deframeReceivedData: "
                  "Checksum error (overall len: %d, data len:%d): our (%d) != received(%d)",
                  overall_length, data_length, our_checksum, received_checksum);

        deframed_data.clear();
        return false;
    }
    
//     qDebug("TransportLayerIface:deframeReceivedData: "
//            "received real data (%d), overall (%d)",
//            deframed_data.count(), overall_length);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
