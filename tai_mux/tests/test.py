import unittest
import subprocess as sp
import threading
import os
import time
import taish
import asyncio

TAI_TEST_MODULE_LOCATION = os.environ.get("TAI_TEST_MODULE_LOCATION", "")
if not TAI_TEST_MODULE_LOCATION:
    TAI_TEST_MODULE_LOCATION = "0"

TAI_TEST_TAISH_SERVER_ADDRESS = os.environ.get("TAI_TEST_TAISH_SERVER_ADDRESS", "")
if not TAI_TEST_TAISH_SERVER_ADDRESS:
    TAI_TEST_TAISH_SERVER_ADDRESS = taish.DEFAULT_SERVER_ADDRESS

TAI_TEST_TAISH_SERVER_PORT = os.environ.get("TAI_TEST_TAISH_SERVER_PORT", "")
if not TAI_TEST_TAISH_SERVER_PORT:
    TAI_TEST_TAISH_SERVER_PORT = taish.DEFAULT_SERVER_PORT

TAI_TEST_NO_LOCAL_TAISH_SERVER = (
    True if os.environ.get("TAI_TEST_NO_LOCAL_TAISH_SERVER", "") else False
)


def output_reader(proc):
    for line in iter(proc.stdout.readline, b""):
        print("taish-server: {}".format(line.decode("utf-8")), end="")


class TestTAI(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        if TAI_TEST_NO_LOCAL_TAISH_SERVER:
            return
        proc = sp.Popen(["taish_server", "-v", "-n"], stderr=sp.STDOUT, stdout=sp.PIPE)
        self.d = threading.Thread(target=output_reader, args=(proc,))
        self.d.start()
        self.proc = proc
        time.sleep(1)  # wait for the server to be ready

    def tearDown(self):
        if TAI_TEST_NO_LOCAL_TAISH_SERVER:
            return
        self.proc.terminate()
        self.proc.wait(timeout=0.2)
        self.d.join()
        self.proc.stdout.close()

    async def test_create_and_remove(self):
        cli = taish.AsyncClient(
            TAI_TEST_TAISH_SERVER_ADDRESS, TAI_TEST_TAISH_SERVER_PORT
        )
        m = await cli.list()
        v = await asyncio.gather(*(cli.create_module(k) for k in m.keys()))
        await asyncio.gather(*(cli.remove(v.oid) for v in v))
        cli.close()
