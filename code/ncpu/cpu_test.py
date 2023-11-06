#!/usr/bin/env python3

import unittest

from cpu import CPU, STACK, State, DATA, Memory


class CPUTest(unittest.TestCase):
    def test_init(self):
        sentinel = list()
        cpu = CPU(sentinel)

        self.assertEqual(STACK, cpu.sp)
        self.assertEqual(1, len(cpu.states))
        self.assertEqual(sentinel, cpu.instructions)
        self.assertEqual(None, cpu.data)

    def test_init_data(self):
        sentinel = list()
        cpu = CPU(sentinel, data=sentinel)

        self.assertEqual(STACK, cpu.sp)
        self.assertEqual(1, len(cpu.states))
        self.assertEqual(sentinel, cpu.instructions)
        self.assertEqual(sentinel, cpu.data)

    def test_load(self):
        cpu = CPU(list())

        data = {}
        for n, char in enumerate("hello"):
            data[n] = ord(char)
        cpu._load(data)

        self.assertEqual(None, cpu.data)
        self.assertEqual(cpu.memory[1 + DATA], data[1])
        self.assertEqual(5, len(cpu.memory))

    def test_rewrite_labels(self):
        program = [
                ('fake', [(1, 'tl'), ('sp', 'm')]),
                ('fake', [('bp', 'r'), (1, 'tl')]),
                ('fake', [(1, 'tl'), (1, 'tl')]),
                ('fake', [('bp', 'r'), ('sp', 'm')]),
                ('fake', [(2, 'dl'), ('sp', 'm')]),
                ('fake', [('bp', 'r'), (2, 'dl')]),
                ('fake', [(2, 'dl'), (2, 'dl')]),
                ('fake', [(2, 'dl'), (1, 'tl')]),
                ('fake', [(1, 'tl'), (2, 'dl')]),
                ]

        rewrite = [
                ('fake', [(0x5501, 'i'), ('sp', 'm')]),
                ('fake', [('bp', 'r'), (0x5501, 'i')]),
                ('fake', [(0x5501, 'i'), (0x5501, 'i')]),
                ('fake', [('bp', 'r'), ('sp', 'm')]),
                ('fake', [(0x6b02, 'i'), ('sp', 'm')]),
                ('fake', [('bp', 'r'), (0x6b02, 'i')]),
                ('fake', [(0x6b02, 'i'), (0x6b02, 'i')]),
                ('fake', [(0x6b02, 'i'), (0x5501, 'i')]),
                ('fake', [(0x5501, 'i'), (0x6b02, 'i')])
                ]

        cpu = CPU([])
        self.assertEqual(rewrite, cpu._rewrite_labels(program))

    def test_get_operand_value(self):
        state = State(registers={'ax': 0x22})
        cpu = CPU([], state=state)

        self.assertEqual(0x22, cpu._get_operand_value('ax', 'r'))

        cpu._write_memory(0x22, 0x1234)
        self.assertEqual(0x1234, cpu._get_operand_value('ax', 'm'))
        self.assertEqual(0x0012, cpu._get_operand_value('ax+1', 'm'))
        self.assertEqual(0x3400, cpu._get_operand_value('ax-1', 'm'))

        self.assertEqual(0x22, cpu._get_operand_value(0x22, 'i'))

    def test_set_operand_value(self):
        pass

    def test_read_memory(self):
        pass

    def test_write_memory(self):
        pass

    def test_memory_operand(self):
        pass

    def test_op_mov(self):
        # confirm raises, test supported combinations
        cpu = CPU([])

        cpu.op_mov(('ax', 'r'), (STACK, 'i'))
        self.assertEqual(STACK, cpu.registers['ax'])

        cpu.op_mov(('bx', 'r'), ('ax', 'r'))
        self.assertEqual(STACK, cpu.registers['bx'])

        cpu.op_mov(('ax', 'm'), ('ax', 'r'))
        self.assertEqual(STACK, cpu._read_memory(STACK))

        with self.assertRaises(Exception):
            cpu.op_mov(('ax', 'm'), ('ax', 'm'))

        with self.assertRaises(Exception):
            cpu.op_mov(('0x1', 'i'), ('ax', 'r'))

        with self.assertRaises(Exception):
            state = State(registers={'ax': 0x1234, 'bx': 0x22})
            cpu = CPU([], state=state)
            cpu.op_mov(('ax', 'm'), ('bx', 'r'))

    def test_op_mul(self):
        state = State(registers={'ax': 0x10, 'bx': 0x3, 'cx': 0x4},
                      memory=Memory({0x4: 0xef, 0x5: 0xbe}))

        # For fun, go-style tests:
        class Test:
            def __init__(self, multiplier, ax, dx, cf, of):
                self.multiplier = multiplier
                self.ax = ax
                self.dx = dx
                self.cf = cf
                self.of = of

            def __repr__(self):
                return str(self.__dict__)

        for test in (
                Test(('bx', 'r'), 0x30, 0, 0, 0),
                Test(('cx', 'm'), 0xeef0, 0xb, 1, 1),
                ):

            with self.subTest(test=test):
                cpu = CPU([], state=state.copy())
                cpu.op_mul(test.multiplier)
                self.assertEqual(test.ax, cpu.registers['ax'])
                self.assertEqual(test.dx, cpu.registers['dx'])
                self.assertEqual(test.cf, cpu.flags['cf'])
                self.assertEqual(test.of, cpu.flags['of'])

        with self.assertRaises(Exception):
            cpu = CPU([])
            cpu.op_mul((0x1, 'i'))  # invalid optype

    def test_op_add(self):
        state = State(registers={'ax': 0x2, 'bx': 0x3, 'cx': 0x4},
                      memory=Memory({0x4: 0x34, 0x5: 0x12}))

        class Test:
            def __init__(self, dest, src, res):
                self.dest = dest
                self.src = src
                self.res = res

            def __repr__(self):
                return str(self.__dict__)

        for test in (
                Test(('bx', 'r'), (0x2, 'i'), 0x5),
                Test(('ax', 'r'), ('ax', 'r'), 0x4),
                Test(('bx', 'r'), ('cx', 'm'), 0x1237),
                ):

            with self.subTest(test=test):
                cpu = CPU([], state=state.copy())
                cpu.op_add(test.dest, test.src)
                register, _ = test.dest
                self.assertEqual(test.res, cpu.registers[register])

    def test_op_sub(self):
        state = State(registers={'ax': 0x2, 'bx': 0x2468, 'cx': 0x4},
                      memory=Memory({0x4: 0x34, 0x5: 0x12}))

        class Test:
            def __init__(self, dest, src, res):
                self.dest = dest
                self.src = src
                self.res = res

            def __repr__(self):
                return str(self.__dict__)

        for test in (
                Test(('bx', 'r'), (0x2, 'i'), 0x2466),
                Test(('ax', 'r'), ('ax', 'r'), 0x0),
                Test(('bx', 'r'), ('ax', 'r'), 0x2466),
                Test(('bx', 'r'), ('cx', 'm'), 0x1234),
                ):

            with self.subTest(test=test):
                cpu = CPU([], state=state.copy())
                cpu.op_sub(test.dest, test.src)
                register, _ = test.dest
                self.assertEqual(test.res, cpu.registers[register])

    def test_op_cmp(self):
        state = State(registers={'ax': 0x2, 'bx': 0x1234, 'cx': 0x4},
                      memory=Memory({0x4: 0x34, 0x5: 0x12}))

        class Test:
            def __init__(self, dest, src, res):
                self.dest = dest
                self.src = src
                self.res = res

            def __repr__(self):
                return str(self.__dict__)

        for test in (
                Test(('ax', 'r'), (0x2, 'i'), 1),
                Test(('bx', 'r'), (0x2, 'i'), 0),
                Test(('ax', 'r'), ('bx', 'r'), 0),
                Test(('ax', 'r'), ('ax', 'r'), 1),
                Test(('ax', 'r'), ('cx', 'm'), 0),
                Test(('bx', 'r'), ('cx', 'm'), 1),
                ):

            with self.subTest(test=test):
                cpu = CPU([], state=state.copy())
                cpu.op_cmp(test.dest, test.src)
                self.assertEqual(test.res, cpu.flags['zf'])

        with self.assertRaises(Exception):
            cpu = CPU([], state=state.copy())
            cpu.op_cmp('bx', 'm'), ('ax', 'm')

        with self.assertRaises(Exception):
            cpu = CPU([], state=state.copy())
            cpu.op_cmp(1, 'i'), ('ax', 'm')

    def test_op_jmp(self):
        state = State(registers={'ax': 0x2, 'bx': 0x5517, 'cx': 0x4},
                      memory=Memory({0x4: 0x10, 0x5: 0x55}))

        class Test:
            def __init__(self, dest, ip):
                self.dest = dest
                self.ip = ip

            def __repr__(self):
                return str(self.__dict__)

        for test in (
                Test(('bx', 'r'), 0x5517),
                Test(('cx', 'm'), 0x5510)
                ):

            with self.subTest(test=test):
                cpu = CPU([], state=state.copy())
                cpu.op_jmp(test.dest)
                self.assertEqual(test.ip, cpu.ip)

    def test_op_jne(self):
        state = State(registers={'ax': 0x2, 'bx': 0x5517, 'cx': 0x4},
                      memory=Memory({0x4: 0x10, 0x5: 0x55}))

        class Test:
            def __init__(self, dest, zf, ip):
                self.dest = dest
                self.zf = zf
                self.ip = ip

            def __repr__(self):
                return str(self.__dict__)

        for test in (
                Test(('bx', 'r'), 0, 0x5517),
                Test(('bx', 'r'), 1, 0),
                Test(('cx', 'm'), 0, 0x5510),
                Test(('cx', 'm'), 1, 0)
                ):

            with self.subTest(test=test):
                cpu = CPU([], state=state.copy())
                cpu.flags['zf'] = test.zf
                cpu.op_jne(test.dest)
                self.assertEqual(test.ip, cpu.ip)

    def test_op_je(self):
        state = State(registers={'ax': 0x2, 'bx': 0x5517, 'cx': 0x4},
                      memory=Memory({0x4: 0x10, 0x5: 0x55}))

        class Test:
            def __init__(self, dest, zf, ip):
                self.dest = dest
                self.zf = zf
                self.ip = ip

            def __repr__(self):
                return str(self.__dict__)

        for test in (
                Test(('bx', 'r'), 1, 0x5517),
                Test(('bx', 'r'), 0, 0),
                Test(('cx', 'm'), 1, 0x5510),
                Test(('cx', 'm'), 0, 0)
                ):

            with self.subTest(test=test):
                cpu = CPU([], state=state.copy())
                cpu.flags['zf'] = test.zf
                cpu.op_je(test.dest)
                self.assertEqual(test.ip, cpu.ip)

    def test_op_push(self):
        state = State(registers={
            'ax': 0x2, 'bx': 0x5517, 'cx': 0x4, 'sp': STACK},
                      memory=Memory({0x4: 0x10, 0x5: 0x55}))

        class Test:
            def __init__(self, src, res):
                self.src = src
                self.res = res

            def __repr__(self):
                return f'push {self.src} expect {self.res}'

        for test in (
                Test(('ax', 'r'), 0x2),
                Test(('cx', 'm'), 0x5510),
                Test((0x1234, 'i'), 0x1234),
                ):

            with self.subTest(test=test):
                cpu = CPU([], state=state.copy())
                sp = cpu.sp
                cpu.op_push(test.src)
                self.assertEqual(sp - 2, cpu.sp)
                self.assertEqual(test.res, cpu._read_memory(cpu.sp))

    def test_op_pop(self):
        pass

    def test_op_call(self):
        pass

    def test_op_ret(self):
        pass
