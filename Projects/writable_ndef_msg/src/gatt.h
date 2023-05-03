/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_BLUETOOTH_GATT_H_
#define ZEPHYR_INCLUDE_BLUETOOTH_GATT_H_
 
#include <stdint.h>
#include <stddef.h>
 
#include <sys/types.h>
 
#include <zephyr/sys/slist.h>
#include <zephyr/sys/util.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/att.h>
 
#ifdef __cplusplus
extern "C" {
#endif
 
enum bt_gatt_perm {
        BT_GATT_PERM_NONE = 0,
 
        BT_GATT_PERM_READ = BIT(0),
 
        BT_GATT_PERM_WRITE = BIT(1),
 
        BT_GATT_PERM_READ_ENCRYPT = BIT(2),
 
        BT_GATT_PERM_WRITE_ENCRYPT = BIT(3),
 
        BT_GATT_PERM_READ_AUTHEN = BIT(4),
 
        BT_GATT_PERM_WRITE_AUTHEN = BIT(5),
 
        BT_GATT_PERM_PREPARE_WRITE = BIT(6),
 
        BT_GATT_PERM_READ_LESC = BIT(7),
 
        BT_GATT_PERM_WRITE_LESC = BIT(8),
};
 
#define BT_GATT_ERR(_att_err) (-(_att_err))
 
enum {
        BT_GATT_WRITE_FLAG_PREPARE = BIT(0),
 
        BT_GATT_WRITE_FLAG_CMD = BIT(1),
 
        BT_GATT_WRITE_FLAG_EXECUTE = BIT(2),
};
 
/* Forward declaration of GATT Attribute structure */
struct bt_gatt_attr;
 
typedef ssize_t (*bt_gatt_attr_read_func_t)(struct bt_conn *conn,
                                            const struct bt_gatt_attr *attr,
                                            void *buf, uint16_t len,
                                            uint16_t offset);
 
typedef ssize_t (*bt_gatt_attr_write_func_t)(struct bt_conn *conn,
                                             const struct bt_gatt_attr *attr,
                                             const void *buf, uint16_t len,
                                             uint16_t offset, uint8_t flags);
 
struct bt_gatt_attr {
        const struct bt_uuid *uuid;
        bt_gatt_attr_read_func_t read;
        bt_gatt_attr_write_func_t write;
        void *user_data;
        uint16_t handle;
        uint16_t perm;
};
 
struct bt_gatt_service_static {
        const struct bt_gatt_attr *attrs;
        size_t attr_count;
};
 
struct bt_gatt_service {
        struct bt_gatt_attr *attrs;
        size_t attr_count;
 
        sys_snode_t node;
};
 
struct bt_gatt_service_val {
        const struct bt_uuid *uuid;
        uint16_t end_handle;
};
 
struct bt_gatt_include {
        const struct bt_uuid *uuid;
        uint16_t start_handle;
        uint16_t end_handle;
};
 
struct bt_gatt_cb {
        void (*att_mtu_updated)(struct bt_conn *conn, uint16_t tx, uint16_t rx);
 
        sys_snode_t node;
};
 
#define BT_GATT_CHRC_BROADCAST                  0x01
#define BT_GATT_CHRC_READ                       0x02
#define BT_GATT_CHRC_WRITE_WITHOUT_RESP         0x04
#define BT_GATT_CHRC_WRITE                      0x08
#define BT_GATT_CHRC_NOTIFY                     0x10
#define BT_GATT_CHRC_INDICATE                   0x20
#define BT_GATT_CHRC_AUTH                       0x40
#define BT_GATT_CHRC_EXT_PROP                   0x80
 
struct bt_gatt_chrc {
        const struct bt_uuid *uuid;
        uint16_t value_handle;
        uint8_t properties;
};
 
#define BT_GATT_CEP_RELIABLE_WRITE              0x0001
#define BT_GATT_CEP_WRITABLE_AUX                0x0002
 
struct bt_gatt_cep {
        uint16_t properties;
};
 
#define BT_GATT_CCC_NOTIFY                      0x0001
#define BT_GATT_CCC_INDICATE                    0x0002
 
struct bt_gatt_ccc {
        uint16_t flags;
};
 
#define BT_GATT_SCC_BROADCAST                   0x0001
 
struct bt_gatt_scc {
        uint16_t flags;
};
 
struct bt_gatt_cpf {
        uint8_t format;
        int8_t exponent;
        uint16_t unit;
        uint8_t name_space;
        uint16_t description;
};
 
void bt_gatt_cb_register(struct bt_gatt_cb *cb);
 
int bt_gatt_service_register(struct bt_gatt_service *svc);
 
int bt_gatt_service_unregister(struct bt_gatt_service *svc);
 
bool bt_gatt_service_is_registered(const struct bt_gatt_service *svc);
 
enum {
        BT_GATT_ITER_STOP = 0,
        BT_GATT_ITER_CONTINUE,
};
 
typedef uint8_t (*bt_gatt_attr_func_t)(const struct bt_gatt_attr *attr,
                                       uint16_t handle,
                                       void *user_data);
 
void bt_gatt_foreach_attr_type(uint16_t start_handle, uint16_t end_handle,
                               const struct bt_uuid *uuid,
                               const void *attr_data, uint16_t num_matches,
                               bt_gatt_attr_func_t func,
                               void *user_data);
 
static inline void bt_gatt_foreach_attr(uint16_t start_handle, uint16_t end_handle,
                                        bt_gatt_attr_func_t func,
                                        void *user_data)
{
        bt_gatt_foreach_attr_type(start_handle, end_handle, NULL, NULL, 0, func,
                                  user_data);
}
 
struct bt_gatt_attr *bt_gatt_attr_next(const struct bt_gatt_attr *attr);
 
struct bt_gatt_attr *bt_gatt_find_by_uuid(const struct bt_gatt_attr *attr,
                                          uint16_t attr_count,
                                          const struct bt_uuid *uuid);
 
uint16_t bt_gatt_attr_get_handle(const struct bt_gatt_attr *attr);
 
uint16_t bt_gatt_attr_value_handle(const struct bt_gatt_attr *attr);
 
ssize_t bt_gatt_attr_read(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          void *buf, uint16_t buf_len, uint16_t offset,
                          const void *value, uint16_t value_len);
 
ssize_t bt_gatt_attr_read_service(struct bt_conn *conn,
                                  const struct bt_gatt_attr *attr,
                                  void *buf, uint16_t len, uint16_t offset);
 
#define BT_GATT_SERVICE_DEFINE(_name, ...)                              \
        const struct bt_gatt_attr attr_##_name[] = { __VA_ARGS__ };     \
        const STRUCT_SECTION_ITERABLE(bt_gatt_service_static, _name) =  \
                                        BT_GATT_SERVICE(attr_##_name)
 
#define _BT_GATT_ATTRS_ARRAY_DEFINE(n, _instances, _attrs_def)  \
        static struct bt_gatt_attr attrs_##n[] = _attrs_def(_instances[n])
 
#define _BT_GATT_SERVICE_ARRAY_ITEM(_n, _) BT_GATT_SERVICE(attrs_##_n)
 
#define BT_GATT_SERVICE_INSTANCE_DEFINE(                                 \
        _name, _instances, _instance_num, _attrs_def)                    \
        BUILD_ASSERT(ARRAY_SIZE(_instances) == _instance_num,            \
                "The number of array elements does not match its size"); \
        LISTIFY(_instance_num, _BT_GATT_ATTRS_ARRAY_DEFINE, (;),         \
                _instances, _attrs_def);                                 \
        static struct bt_gatt_service _name[] = {                        \
                LISTIFY(_instance_num, _BT_GATT_SERVICE_ARRAY_ITEM, (,)) \
        }
 
#define BT_GATT_SERVICE(_attrs)                                         \
{                                                                       \
        .attrs = _attrs,                                                \
        .attr_count = ARRAY_SIZE(_attrs),                               \
}
 
#define BT_GATT_PRIMARY_SERVICE(_service)                               \
        BT_GATT_ATTRIBUTE(BT_UUID_GATT_PRIMARY, BT_GATT_PERM_READ,      \
                         bt_gatt_attr_read_service, NULL, _service)
 
#define BT_GATT_SECONDARY_SERVICE(_service)                             \
        BT_GATT_ATTRIBUTE(BT_UUID_GATT_SECONDARY, BT_GATT_PERM_READ,    \
                         bt_gatt_attr_read_service, NULL, _service)
 
ssize_t bt_gatt_attr_read_included(struct bt_conn *conn,
                                   const struct bt_gatt_attr *attr,
                                   void *buf, uint16_t len, uint16_t offset);
 
#define BT_GATT_INCLUDE_SERVICE(_service_incl)                          \
        BT_GATT_ATTRIBUTE(BT_UUID_GATT_INCLUDE, BT_GATT_PERM_READ,      \
                          bt_gatt_attr_read_included, NULL, _service_incl)
 
ssize_t bt_gatt_attr_read_chrc(struct bt_conn *conn,
                               const struct bt_gatt_attr *attr, void *buf,
                               uint16_t len, uint16_t offset);
 
#define BT_GATT_CHRC_INIT(_uuid, _handle, _props) \
{                                                 \
        .uuid = _uuid,                            \
        .value_handle = _handle,                  \
        .properties = _props,                     \
}
 
#define BT_GATT_CHARACTERISTIC(_uuid, _props, _perm, _read, _write, _user_data) \
        BT_GATT_ATTRIBUTE(BT_UUID_GATT_CHRC, BT_GATT_PERM_READ,                 \
                          bt_gatt_attr_read_chrc, NULL,                         \
                          ((struct bt_gatt_chrc[]) {                            \
                                BT_GATT_CHRC_INIT(_uuid, 0U, _props),           \
                                                   })),                         \
        BT_GATT_ATTRIBUTE(_uuid, _perm, _read, _write, _user_data)
 
#if defined(CONFIG_BT_SETTINGS_CCC_LAZY_LOADING)
        #define BT_GATT_CCC_MAX (CONFIG_BT_MAX_CONN)
#elif defined(CONFIG_BT_CONN)
        #define BT_GATT_CCC_MAX (CONFIG_BT_MAX_PAIRED + CONFIG_BT_MAX_CONN)
#else
        #define BT_GATT_CCC_MAX 0
#endif
 
struct bt_gatt_ccc_cfg {
        uint8_t id;
        bt_addr_le_t peer;
        uint16_t value;
};
 
struct _bt_gatt_ccc {
        struct bt_gatt_ccc_cfg cfg[BT_GATT_CCC_MAX];
 
        uint16_t value;
 
        void (*cfg_changed)(const struct bt_gatt_attr *attr, uint16_t value);
 
        ssize_t (*cfg_write)(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr, uint16_t value);
 
        bool (*cfg_match)(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr);
};
 
ssize_t bt_gatt_attr_read_ccc(struct bt_conn *conn,
                              const struct bt_gatt_attr *attr, void *buf,
                              uint16_t len, uint16_t offset);
 
ssize_t bt_gatt_attr_write_ccc(struct bt_conn *conn,
                               const struct bt_gatt_attr *attr, const void *buf,
                               uint16_t len, uint16_t offset, uint8_t flags);
 
 
#define BT_GATT_CCC_INITIALIZER(_changed, _write, _match) \
        {                                            \
                .cfg = {},                           \
                .cfg_changed = _changed,             \
                .cfg_write = _write,                 \
                .cfg_match = _match,                 \
        }
 
#define BT_GATT_CCC_MANAGED(_ccc, _perm)                                \
        BT_GATT_ATTRIBUTE(BT_UUID_GATT_CCC, _perm,                      \
                        bt_gatt_attr_read_ccc, bt_gatt_attr_write_ccc,  \
                        _ccc)
 
#define BT_GATT_CCC(_changed, _perm)                            \
        BT_GATT_CCC_MANAGED(((struct _bt_gatt_ccc[])                    \
                {BT_GATT_CCC_INITIALIZER(_changed, NULL, NULL)}), _perm)
 
ssize_t bt_gatt_attr_read_cep(struct bt_conn *conn,
                              const struct bt_gatt_attr *attr, void *buf,
                              uint16_t len, uint16_t offset);
 
#define BT_GATT_CEP(_value)                                             \
        BT_GATT_DESCRIPTOR(BT_UUID_GATT_CEP, BT_GATT_PERM_READ,         \
                          bt_gatt_attr_read_cep, NULL, (void *)_value)
 
ssize_t bt_gatt_attr_read_cud(struct bt_conn *conn,
                              const struct bt_gatt_attr *attr, void *buf,
                              uint16_t len, uint16_t offset);
 
#define BT_GATT_CUD(_value, _perm)                                      \
        BT_GATT_DESCRIPTOR(BT_UUID_GATT_CUD, _perm, bt_gatt_attr_read_cud, \
                           NULL, (void *)_value)
 
ssize_t bt_gatt_attr_read_cpf(struct bt_conn *conn,
                              const struct bt_gatt_attr *attr, void *buf,
                              uint16_t len, uint16_t offset);
 
#define BT_GATT_CPF(_value)                                             \
        BT_GATT_DESCRIPTOR(BT_UUID_GATT_CPF, BT_GATT_PERM_READ,         \
                          bt_gatt_attr_read_cpf, NULL, (void *)_value)
 
#define BT_GATT_DESCRIPTOR(_uuid, _perm, _read, _write, _user_data)     \
        BT_GATT_ATTRIBUTE(_uuid, _perm, _read, _write, _user_data)
 
#define BT_GATT_ATTRIBUTE(_uuid, _perm, _read, _write, _user_data)      \
{                                                                       \
        .uuid = _uuid,                                                  \
        .read = _read,                                                  \
        .write = _write,                                                \
        .user_data = _user_data,                                        \
        .handle = 0,                                                    \
        .perm = _perm,                                                  \
}
 
typedef void (*bt_gatt_complete_func_t) (struct bt_conn *conn, void *user_data);
 
struct bt_gatt_notify_params {
        const struct bt_uuid *uuid;
        const struct bt_gatt_attr *attr;
        const void *data;
        uint16_t len;
        bt_gatt_complete_func_t func;
        void *user_data;
#if defined(CONFIG_BT_EATT)
        enum bt_att_chan_opt chan_opt;
#endif /* CONFIG_BT_EATT */
};
 
int bt_gatt_notify_cb(struct bt_conn *conn,
                      struct bt_gatt_notify_params *params);
 
int bt_gatt_notify_multiple(struct bt_conn *conn,
                            uint16_t num_params,
                            struct bt_gatt_notify_params params[]);
 
static inline int bt_gatt_notify(struct bt_conn *conn,
                                 const struct bt_gatt_attr *attr,
                                 const void *data, uint16_t len)
{
        struct bt_gatt_notify_params params;
 
        memset(&params, 0, sizeof(params));
 
        params.attr = attr;
        params.data = data;
        params.len = len;
#if defined(CONFIG_BT_EATT)
        params.chan_opt = BT_ATT_CHAN_OPT_NONE;
#endif /* CONFIG_BT_EATT */
 
        return bt_gatt_notify_cb(conn, &params);
}
 
static inline int bt_gatt_notify_uuid(struct bt_conn *conn,
                                      const struct bt_uuid *uuid,
                                      const struct bt_gatt_attr *attr,
                                      const void *data, uint16_t len)
{
        struct bt_gatt_notify_params params;
 
        memset(&params, 0, sizeof(params));
 
        params.uuid = uuid;
        params.attr = attr;
        params.data = data;
        params.len = len;
#if defined(CONFIG_BT_EATT)
        params.chan_opt = BT_ATT_CHAN_OPT_NONE;
#endif /* CONFIG_BT_EATT */
 
        return bt_gatt_notify_cb(conn, &params);
}
 
/* Forward declaration of the bt_gatt_indicate_params structure */
struct bt_gatt_indicate_params;
 
typedef void (*bt_gatt_indicate_func_t)(struct bt_conn *conn,
                                        struct bt_gatt_indicate_params *params,
                                        uint8_t err);
 
typedef void (*bt_gatt_indicate_params_destroy_t)(
                struct bt_gatt_indicate_params *params);
 
struct bt_gatt_indicate_params {
        const struct bt_uuid *uuid;
        const struct bt_gatt_attr *attr;
        bt_gatt_indicate_func_t func;
        bt_gatt_indicate_params_destroy_t destroy;
        const void *data;
        uint16_t len;
        uint8_t _ref;
#if defined(CONFIG_BT_EATT)
        enum bt_att_chan_opt chan_opt;
#endif /* CONFIG_BT_EATT */
};
 
int bt_gatt_indicate(struct bt_conn *conn,
                     struct bt_gatt_indicate_params *params);
 
 
bool bt_gatt_is_subscribed(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr, uint16_t ccc_type);
 
uint16_t bt_gatt_get_mtu(struct bt_conn *conn);
 
struct bt_gatt_exchange_params {
        void (*func)(struct bt_conn *conn, uint8_t err,
                     struct bt_gatt_exchange_params *params);
};
 
int bt_gatt_exchange_mtu(struct bt_conn *conn,
                         struct bt_gatt_exchange_params *params);
 
struct bt_gatt_discover_params;
 
typedef uint8_t (*bt_gatt_discover_func_t)(struct bt_conn *conn,
                                        const struct bt_gatt_attr *attr,
                                        struct bt_gatt_discover_params *params);
 
enum {
        BT_GATT_DISCOVER_PRIMARY,
        BT_GATT_DISCOVER_SECONDARY,
        BT_GATT_DISCOVER_INCLUDE,
        BT_GATT_DISCOVER_CHARACTERISTIC,
        BT_GATT_DISCOVER_DESCRIPTOR,
        BT_GATT_DISCOVER_ATTRIBUTE,
        BT_GATT_DISCOVER_STD_CHAR_DESC,
};
 
struct bt_gatt_discover_params {
        const struct bt_uuid *uuid;
        bt_gatt_discover_func_t func;
        union {
                struct {
                        uint16_t attr_handle;
                        uint16_t start_handle;
                        uint16_t end_handle;
                } _included;
                uint16_t start_handle;
        };
        uint16_t end_handle;
        uint8_t type;
#if defined(CONFIG_BT_GATT_AUTO_DISCOVER_CCC)
        struct bt_gatt_subscribe_params *sub_params;
#endif /* defined(CONFIG_BT_GATT_AUTO_DISCOVER_CCC) */
#if defined(CONFIG_BT_EATT)
        enum bt_att_chan_opt chan_opt;
#endif /* CONFIG_BT_EATT */
};
 
int bt_gatt_discover(struct bt_conn *conn,
                     struct bt_gatt_discover_params *params);
 
struct bt_gatt_read_params;
 
typedef uint8_t (*bt_gatt_read_func_t)(struct bt_conn *conn, uint8_t err,
                                    struct bt_gatt_read_params *params,
                                    const void *data, uint16_t length);
 
struct bt_gatt_read_params {
        bt_gatt_read_func_t func;
        size_t handle_count;
        union {
                struct {
                        uint16_t handle;
                        uint16_t offset;
                } single;
                struct {
                        uint16_t *handles;
                        bool variable;
                } multiple;
                struct {
                        uint16_t start_handle;
                        uint16_t end_handle;
                        const struct bt_uuid *uuid;
                } by_uuid;
        };
#if defined(CONFIG_BT_EATT)
        enum bt_att_chan_opt chan_opt;
#endif /* CONFIG_BT_EATT */
};
 
int bt_gatt_read(struct bt_conn *conn, struct bt_gatt_read_params *params);
 
struct bt_gatt_write_params;
 
typedef void (*bt_gatt_write_func_t)(struct bt_conn *conn, uint8_t err,
                                     struct bt_gatt_write_params *params);
 
struct bt_gatt_write_params {
        bt_gatt_write_func_t func;
        uint16_t handle;
        uint16_t offset;
        const void *data;
        uint16_t length;
#if defined(CONFIG_BT_EATT)
        enum bt_att_chan_opt chan_opt;
#endif /* CONFIG_BT_EATT */
};
 
int bt_gatt_write(struct bt_conn *conn, struct bt_gatt_write_params *params);
 
int bt_gatt_write_without_response_cb(struct bt_conn *conn, uint16_t handle,
                                      const void *data, uint16_t length,
                                      bool sign, bt_gatt_complete_func_t func,
                                      void *user_data);
 
static inline int bt_gatt_write_without_response(struct bt_conn *conn,
                                                 uint16_t handle, const void *data,
                                                 uint16_t length, bool sign)
{
        return bt_gatt_write_without_response_cb(conn, handle, data, length,
                                                 sign, NULL, NULL);
}
 
struct bt_gatt_subscribe_params;
 
typedef uint8_t (*bt_gatt_notify_func_t)(struct bt_conn *conn,
                                      struct bt_gatt_subscribe_params *params,
                                      const void *data, uint16_t length);
 
typedef void (*bt_gatt_subscribe_func_t)(struct bt_conn *conn, uint8_t err,
                                         struct bt_gatt_subscribe_params *params);
 
enum {
        BT_GATT_SUBSCRIBE_FLAG_VOLATILE,
 
        BT_GATT_SUBSCRIBE_FLAG_NO_RESUB,
 
        BT_GATT_SUBSCRIBE_FLAG_WRITE_PENDING,
 
        BT_GATT_SUBSCRIBE_FLAG_SENT,
 
        BT_GATT_SUBSCRIBE_NUM_FLAGS
};
 
struct bt_gatt_subscribe_params {
        bt_gatt_notify_func_t notify;
        bt_gatt_subscribe_func_t subscribe;
 
        bt_gatt_write_func_t write;
        uint16_t value_handle;
        uint16_t ccc_handle;
#if defined(CONFIG_BT_GATT_AUTO_DISCOVER_CCC)
        uint16_t end_handle;
        struct bt_gatt_discover_params *disc_params;
#endif /* CONFIG_BT_GATT_AUTO_DISCOVER_CCC */
        uint16_t value;
#if defined(CONFIG_BT_SMP)
        bt_security_t min_security;
#endif
        ATOMIC_DEFINE(flags, BT_GATT_SUBSCRIBE_NUM_FLAGS);
 
        sys_snode_t node;
#if defined(CONFIG_BT_EATT)
        enum bt_att_chan_opt chan_opt;
#endif /* CONFIG_BT_EATT */
};
 
int bt_gatt_subscribe(struct bt_conn *conn,
                      struct bt_gatt_subscribe_params *params);
 
int bt_gatt_resubscribe(uint8_t id, const bt_addr_le_t *peer,
                        struct bt_gatt_subscribe_params *params);
 
int bt_gatt_unsubscribe(struct bt_conn *conn,
                        struct bt_gatt_subscribe_params *params);
 
void bt_gatt_cancel(struct bt_conn *conn, void *params);
 
#ifdef __cplusplus
}
#endif
 
#endif /* ZEPHYR_INCLUDE_BLUETOOTH_GATT_H_ */