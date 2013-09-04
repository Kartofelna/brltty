/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2013 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

#include "prologue.h"

#include <errno.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>

#include "log.h"
#include "io_bluetooth.h"
#include "bluetooth_internal.h"
#include "io_misc.h"

struct BluetoothConnectionExtensionStruct {
  int socket;
  struct sockaddr_rc local;
  struct sockaddr_rc remote;
};

static void
bthMakeAddress (bdaddr_t *address, uint64_t bda) {
  unsigned int index;

  for (index=0; index<BDA_SIZE; index+=1) {
    address->b[index] = bda & 0XFF;
    bda >>= 8;
  }
}

int
bthDiscoverChannel (
  uint8_t *channel, BluetoothConnectionExtension *bcx,
  const void *uuidBytes, size_t uuidLength
) {
  int foundChannel = 0;

#ifdef HAVE_LIBBLUETOOTH
  sdp_session_t *session = sdp_connect(BDADDR_ANY, &bcx->remote.rc_bdaddr, SDP_RETRY_IF_BUSY);

  if (session) {
    uuid_t uuid;
    sdp_list_t *searchList;

    sdp_uuid128_create(&uuid, uuidBytes);
    searchList = sdp_list_append(NULL, &uuid);

    if (searchList) {
      uint32_t attributesRange = 0X0000FFFF;
      sdp_list_t *attributesList = sdp_list_append(NULL, &attributesRange);

      if (attributesList) {
        sdp_list_t *recordList = NULL;
        int queryStatus = sdp_service_search_attr_req(session, searchList,
                                                      SDP_ATTR_REQ_RANGE, attributesList,
                                                      &recordList);

        if (!queryStatus) {
          int stopSearching = 0;
          sdp_list_t *recordElement = recordList;

          while (recordElement) {
            sdp_record_t *record = (sdp_record_t *)recordElement->data;

            if (record) {
              sdp_list_t *protocolsList;

              if (!(sdp_get_access_protos(record, &protocolsList))) {
                sdp_list_t *protocolsElement = protocolsList;

                while (protocolsElement) {
                  sdp_list_t *protocolList = (sdp_list_t *)protocolsElement->data;
                  sdp_list_t *protocolElement = protocolList;

                  while (protocolElement) {
                    sdp_data_t *dataList = (sdp_data_t *)protocolElement->data;
                    sdp_data_t *dataElement = dataList;
                    int uuidProtocol = 0;

                    while (dataElement) {
                      if (SDP_IS_UUID(dataElement->dtd)) {
                        uuidProtocol = sdp_uuid_to_proto(&dataElement->val.uuid);
                      } else if (dataElement->dtd == SDP_UINT8) {
                        if (uuidProtocol == RFCOMM_UUID) {
                          *channel = dataElement->val.uint8;
                          foundChannel = 1;
                          stopSearching = 1;
                        }
                      }

                      if (stopSearching) break;
                      dataElement = dataElement->next;
                    }

                    if (stopSearching) break;
                    protocolElement = protocolElement->next;
                  }

                  sdp_list_free(protocolList, NULL);
                  if (stopSearching) break;
                  protocolsElement = protocolsElement->next;
                }

                sdp_list_free(protocolsList, NULL);
              } else {
                logMallocError();
                stopSearching = 1;
              }

              sdp_record_free(record);
            } else {
              logMallocError();
              stopSearching = 1;
            }

            if (stopSearching) break;
            recordElement = recordElement->next;
          }

          sdp_list_free(recordList, NULL);
        } else {
          logSystemError("sdp_service_search_attr_req");
        }

        sdp_list_free(attributesList, NULL);
      } else {
        logMallocError();
      }

      sdp_list_free(searchList, NULL);
    } else {
      logMallocError();
    }

    sdp_close(session);
  } else {
    logSystemError("sdp_connect");
  }
#endif /* HAVE_LIBBLUETOOTH */

  return foundChannel;
}

BluetoothConnectionExtension *
bthConnect (uint64_t bda, uint8_t channel, int discover, int timeout) {
  BluetoothConnectionExtension *bcx;

  if ((bcx = malloc(sizeof(*bcx)))) {
    memset(bcx, 0, sizeof(*bcx));

    if ((bcx->socket = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) != -1) {
      bcx->local.rc_family = AF_BLUETOOTH;
      bcx->local.rc_channel = 0;
      bacpy(&bcx->local.rc_bdaddr, BDADDR_ANY); /* Any HCI. No support for explicit
                                                 * interface specification yet.
                                                 */

      if (bind(bcx->socket, (struct sockaddr *)&bcx->local, sizeof(bcx->local)) != -1) {
        bcx->remote.rc_family = AF_BLUETOOTH;
        bcx->remote.rc_channel = channel;
        bthMakeAddress(&bcx->remote.rc_bdaddr, bda);

        if (setBlockingIo(bcx->socket, 0)) {
          if (discover) bthDiscoverSerialPortChannel(&channel, bcx);
          bthLogChannel(channel);

          if (connectSocket(bcx->socket, (struct sockaddr *)&bcx->remote, sizeof(bcx->remote), timeout) != -1) {
            return bcx;
          } else if ((errno != EHOSTDOWN) && (errno != EHOSTUNREACH)) {
            logSystemError("RFCOMM connect");
          } else {
            logMessage(LOG_DEBUG, "Bluetooth connect error: %s", strerror(errno));
          }
        }
      } else {
        logSystemError("RFCOMM bind");
      }

      close(bcx->socket);
    } else {
      logSystemError("RFCOMM socket");
    }

    free(bcx);
  } else {
    logMallocError();
  }

  return NULL;
}

void
bthDisconnect (BluetoothConnectionExtension *bcx) {
  close(bcx->socket);
  free(bcx);
}

int
bthAwaitInput (BluetoothConnection *connection, int milliseconds) {
  BluetoothConnectionExtension *bcx = connection->extension;

  return awaitSocketInput(bcx->socket, milliseconds);
}

ssize_t
bthReadData (
  BluetoothConnection *connection, void *buffer, size_t size,
  int initialTimeout, int subsequentTimeout
) {
  BluetoothConnectionExtension *bcx = connection->extension;

  return readSocket(bcx->socket, buffer, size, initialTimeout, subsequentTimeout);
}

ssize_t
bthWriteData (BluetoothConnection *connection, const void *buffer, size_t size) {
  BluetoothConnectionExtension *bcx = connection->extension;

  return writeSocket(bcx->socket, buffer, size);
}

char *
bthObtainDeviceName (uint64_t bda, int timeout) {
  char *name = NULL;

#ifdef HAVE_LIBBLUETOOTH
  int device = hci_get_route(NULL);

  if (device >= 0) {
    int socket = hci_open_dev(device);

    if (socket >= 0) {
      bdaddr_t address;
      char buffer[248];
      int result;

      bthMakeAddress(&address, bda);
      memset(buffer, 0, sizeof(buffer));
      result = hci_read_remote_name(socket, &address, sizeof(buffer), buffer, timeout);

      if (result >= 0) {
        if (!(name = strdup(buffer))) logMallocError();
      } else {
        logSystemError("hci_read_remote_name");
      }

      close(socket);
    } else {
      logSystemError("hci_open_dev");
    }
  } else {
    logSystemError("hci_get_route");
  }
#endif /* HAVE_LIBBLUETOOTH */

  return name;
}
