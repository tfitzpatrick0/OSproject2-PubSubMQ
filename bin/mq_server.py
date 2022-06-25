#!/usr/bin/env python3

''' MQ Server: Message Queue Server

This Message Queue Server supports the following REST API:

    PUT     /topic/$topic               Publish message to $topic.

    GET     /queue/$queue               Retrieve one message from $queue.

    PUT     /subscription/$queue/$topic Subscribe $queue to $topic.
    DELETE  /subscription/$queue/$topic Unsubscribe $queue from $topic.
'''

import collections
import logging
import signal
import socket
import sys
import time

import tornado.gen
import tornado.options
import tornado.web

# Base Handler

class BaseHandler(tornado.web.RequestHandler):
    def write_error(self, status_code, **kwargs):
        self.set_status(status_code)
        try:
            self.write(str(kwargs['exc_info'][1].log_message) + '\n')
        except AttributeError:
            self.write(str(kwargs['exc_info'][1]) + '\n')

    def write_response(self, message):
        self.application.logger.info(message.rstrip())
        self.write(message)

# Topic Handler

class TopicHandler(BaseHandler):
    def put(self, topic):
        ''' Publish message (request body) to each queue that is subscribed to topic. '''
        message     = self.request.body
        subscribers = 0

        for queue, topics in self.application.subscriptions.items():
            if topic in topics:
                self.application.queues[queue].append(message)
                subscribers += 1

        if subscribers:
            self.write('Published message ({} bytes) to {} subscribers of {}\n'.format(
                len(message),
                subscribers,
                topic,
            ))
        else:
            raise tornado.web.HTTPError(404, 'There are no subscribers for topic: {}'.format(topic))

# Queue Handler

class QueueHandler(BaseHandler):
    @tornado.gen.coroutine
    def get(self, queue):
        ''' Retrieve one message from queue (wait until one is available). '''

        if queue not in self.application.queues:
            raise tornado.web.HTTPError(404, 'There is no queue named: {}'.format(queue))

        while not self.application.queues[queue] and not self.request.connection.stream.closed():
            yield tornado.gen.sleep(1)

        if self.application.queues[queue]:
            self.write_response(self.application.queues[queue].pop(0))
        else:
            raise tornado.web.HTTPError(404, 'There are no messages for queue: {}'.format(queue))

# Subscription Handler

class SubscriptionHandler(BaseHandler):
    def put(self, queue, topic):
        ''' Subscribe queue to topic. '''
        try:
            self.application.subscriptions[queue].add(topic)
            if queue not in self.application.queues:
                self.application.queues[queue]
        except KeyError:
            raise tornado.web.HTTPError(404, 'There is no queue named: {}'.format(queue))

        self.write_response('Subscribed queue ({}) to topic ({})\n'.format(queue, topic))

    def delete(self, queue, topic):
        ''' Unsubscribe queue from topic. '''
        try:
            self.application.subscriptions[queue].remove(topic)
        except KeyError:
            raise tornado.web.HTTPError(404, 'There is no queue named: {}'.format(queue))

        self.write_response('Unsubscribed queue ({}) from topic ({})\n'.format(queue, topic))

# Message Queue

class MessageQueue(tornado.web.Application):
    DEFAULT_ADDRESS = '0.0.0.0'
    DEFAULT_PORT    = 9620

    def __init__(self, **settings):
        tornado.web.Application.__init__(self, **settings)

        self.logger        = logging.getLogger()
        self.address       = settings.get('address', self.DEFAULT_ADDRESS)
        self.port          = settings.get('port'   , self.DEFAULT_PORT)
        self.ioloop        = tornado.ioloop.IOLoop.instance()
        self.queues        = collections.defaultdict(list)
        self.subscriptions = collections.defaultdict(set)

        self.add_handlers('.*', (
            ('.*/topic/(.*)'            , TopicHandler),
            ('.*/queue/(.*)'            , QueueHandler),
            ('.*/subscription/(.*)/(.*)', SubscriptionHandler),
        ))

    def run(self):
        try:
            self.listen(self.port, self.address)
        except socket.error as e:
            self.logger.fatal('Unable to listen on {}:{} = {}'.format(self.address, self.port, e))
            sys.exit(1)

        self.ioloop.start()

# Main execution

if __name__ == '__main__':
    tornado.options.define('debug'  , default=False, help='Enable debugging mode')
    tornado.options.define('address', default=MessageQueue.DEFAULT_ADDRESS, help='Address to listen on.')
    tornado.options.define('port'   , default=MessageQueue.DEFAULT_PORT   , help='Port to listen on.')
    tornado.options.parse_command_line()

    signal.signal(signal.SIGTERM, lambda s, e: sys.exit(0))

    message_queue = MessageQueue(**tornado.options.options.as_dict())
    message_queue.run()
