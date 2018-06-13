/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2015-2018 Intel Corporation
 */

#ifndef _QAT_SYM_H_
#define _QAT_SYM_H_

#include <rte_cryptodev_pmd.h>
#include <rte_memzone.h>

#include "qat_common.h"
#include "qat_device.h"
#include "qat_crypto_capabilities.h"

/*
 * This macro rounds up a number to a be a multiple of
 * the alignment when the alignment is a power of 2
 */
#define ALIGN_POW2_ROUNDUP(num, align) \
	(((num) + (align) - 1) & ~((align) - 1))
#define QAT_64_BTYE_ALIGN_MASK (~0x3f)

#define QAT_CSR_HEAD_WRITE_THRESH 32U
/* number of requests to accumulate before writing head CSR */
#define QAT_CSR_TAIL_WRITE_THRESH 32U
/* number of requests to accumulate before writing tail CSR */
#define QAT_CSR_TAIL_FORCE_WRITE_THRESH 256U
/* number of inflights below which no tail write coalescing should occur */

typedef int (*build_request_t)(void *op,
		uint8_t *req, void *op_cookie,
		enum qat_device_gen qat_dev_gen);
/**< Build a request from an op. */

struct qat_sym_session;

/**
 * Structure associated with each queue.
 */
struct qat_queue {
	char		memz_name[RTE_MEMZONE_NAMESIZE];
	void		*base_addr;		/* Base address */
	rte_iova_t	base_phys_addr;		/* Queue physical address */
	uint32_t	head;			/* Shadow copy of the head */
	uint32_t	tail;			/* Shadow copy of the tail */
	uint32_t	modulo;
	uint32_t	msg_size;
	uint16_t	max_inflights;
	uint32_t	queue_size;
	uint8_t		hw_bundle_number;
	uint8_t		hw_queue_number;
	/* HW queue aka ring offset on bundle */
	uint32_t	csr_head;		/* last written head value */
	uint32_t	csr_tail;		/* last written tail value */
	uint16_t	nb_processed_responses;
	/* number of responses processed since last CSR head write */
	uint16_t	nb_pending_requests;
	/* number of requests pending since last CSR tail write */
};

struct qat_qp {
	void			*mmap_bar_addr;
	uint16_t		inflights16;
	struct	qat_queue	tx_q;
	struct	qat_queue	rx_q;
	struct	rte_cryptodev_stats stats;
	struct rte_mempool *op_cookie_pool;
	void **op_cookies;
	uint32_t nb_descriptors;
	enum qat_device_gen qat_dev_gen;
	build_request_t build_request;
} __rte_cache_aligned;


int
qat_sym_build_request(void *in_op, uint8_t *out_msg,
		void *op_cookie, enum qat_device_gen qat_dev_gen);

void qat_sym_stats_get(struct rte_cryptodev *dev,
	struct rte_cryptodev_stats *stats);
void qat_sym_stats_reset(struct rte_cryptodev *dev);

int qat_sym_qp_setup(struct rte_cryptodev *dev, uint16_t queue_pair_id,
	const struct rte_cryptodev_qp_conf *rx_conf, int socket_id,
	struct rte_mempool *session_pool);
int qat_sym_qp_release(struct rte_cryptodev *dev,
	uint16_t queue_pair_id);


uint16_t
qat_sym_pmd_enqueue_op_burst(void *qp, struct rte_crypto_op **ops,
		uint16_t nb_ops);

uint16_t
qat_sym_pmd_dequeue_op_burst(void *qp, struct rte_crypto_op **ops,
		uint16_t nb_ops);

#endif /* _QAT_SYM_H_ */
