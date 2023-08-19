#include "gcp.h"

/**
 * @brief  计算校验
 * @param  msg gcp消息指针
 * @retval 校验
 */
uint8_t gcp_calc_check(gcp_msg_t *msg)
{
    uint16_t i;
    uint8_t sum = 0;
    uint8_t *p = (uint8_t *)msg;

    sum += p[0];
    sum += p[1];
    sum += p[2];
    sum += p[3];
    sum += p[4];
    sum += p[6];
    sum += p[7];

    for (i = 0; i < msg->content_len; i++)
        sum += msg->content[i];

    return sum;
}

/**
 * @brief  gcp消息格式检查
 * @param  msg gcp消息指针
 * @retval 错误码
 *     @arg -1 magic0错误
 *     @arg -2 magic1错误
 *     @arg -3 版本号错误
 *     @arg -4 校验和错误
 *     @arg 0 无错误
 */
int gcp_msg_check(gcp_msg_t *msg)
{
    if (msg->magic0 != GCP_MAGIC0)
        return -1;

    if (msg->magic1 != GCP_MAGIC1)
        return -2;

    if (msg->version != GCP_VERSION)
        return -3;

    if (msg->check != gcp_calc_check(msg))
        return -4;

    return 0;
}

/**
 * @brief    装载gcp消息包
 * @param    gcp_msg 消息包指针
 * @param    type 消息类型
 * @param    src_module 源模块ID
 * @param    dst_module 目的模块ID
 * @param    cmd 命令
 * @param    content 消息内容指针
 * @param    content_len 消息内容长度
 * @retval   消息包长度
 */
uint16_t gcp_msg_pack(gcp_msg_t *gcp_msg,
                     uint8_t type,
                     uint8_t src_module,
                     uint8_t dst_module,
                     uint8_t cmd,
                     uint8_t *content,
                     uint16_t content_len)
{
    uint16_t i;

    gcp_msg->magic0 = GCP_MAGIC0;
    gcp_msg->magic1 = GCP_MAGIC1;
    gcp_msg->content_len = content_len;
    gcp_msg->type = type;
    gcp_msg->version = GCP_VERSION;
    gcp_msg->src_module = src_module;
    gcp_msg->dst_module = dst_module;
    gcp_msg->cmd = cmd;

    for (i = 0; i < content_len; i++)
    {
        gcp_msg->content[i] = content[i];
    }

    gcp_msg->check = gcp_calc_check(gcp_msg);

    return (gcp_msg->content_len + 8);
}
