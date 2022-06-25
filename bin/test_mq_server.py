#!/usr/bin/env python3

import unittest
import requests

# Server Test Case

class ServerTestCase(unittest.TestCase):
    BODY  = 'You win some, you lose some'
    URL   = 'http://localhost:9620'

    def test_00_publish_without_subscribers(self):
        r = requests.put(self.URL + '/topic/_topic', data=self.BODY)
        self.assertEqual(r.status_code  , 404)
        self.assertEqual(r.text.rstrip(), 'There are no subscribers for topic: _topic')
    
    def test_01_retrieve_nonexistent_queue(self):
        r = requests.get(self.URL + '/queue/_queue', data=self.BODY)
        self.assertEqual(r.status_code  , 404)
        self.assertEqual(r.text.rstrip(), 'There is no queue named: _queue')
    
    def test_02_subscribe(self):
        r = requests.put(self.URL + '/subscription/_queue/_topic')
        self.assertEqual(r.status_code  , 200)
        self.assertEqual(r.text.rstrip(), 'Subscribed queue (_queue) to topic (_topic)')
    
    def test_03_publish(self):
        r = requests.put(self.URL + '/topic/_topic', data=self.BODY)
        self.assertEqual(r.status_code  , 200)
        self.assertEqual(
            r.text.rstrip(), 
            'Published message ({} bytes) to 1 subscribers of _topic'.format(len(self.BODY)),
        )

    def test_04_retrieve(self):
        r = requests.get(self.URL + '/queue/_queue', data=self.BODY)
        self.assertEqual(r.status_code  , 200)
        self.assertEqual(r.text.rstrip(), self.BODY)
    
    def test_05_retrieve_timeout(self):
        with self.assertRaises(requests.exceptions.ReadTimeout):
            requests.get(self.URL + '/queue/_queue', data=self.BODY, timeout=2)

        self.test_03_publish()
        self.test_04_retrieve()

    def test_06_unsubscribe(self):
        r = requests.delete(self.URL + '/subscription/_queue/_topic')
        self.assertEqual(r.status_code  , 200)
        self.assertEqual(r.text.rstrip(), 'Unsubscribed queue (_queue) from topic (_topic)')

        self.test_00_publish_without_subscribers()

# Main execution

if __name__ == '__main__':
    unittest.main()
