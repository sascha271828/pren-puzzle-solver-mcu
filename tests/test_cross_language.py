"""
Cross-language test: Python encodes a PuzzleCommand, pipes the
length-prefixed frame to a C decoder, verifies the C side decodes
correctly and sends back an Ack.
"""
import struct
import subprocess
import sys
import os

# Add the proto output to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'pren-puzzle-solver', 'proto'))
import puzzle_pb2


def main():
    print("=== Cross-Language Test ===\n")

    # Build the C helper that reads a frame from stdin and prints decoded values
    base = os.path.join(os.path.dirname(__file__), '..')
    helper_src = os.path.join(os.path.dirname(__file__), 'test_cross_helper.c')
    helper_bin = os.path.join(os.path.dirname(__file__), 'test_cross_helper')

    compile_cmd = [
        'cc', '-o', helper_bin, helper_src,
        os.path.join(base, 'Core/Src/communication/uart_receiver.c'),
        os.path.join(base, 'Core/Src/communication/command_dispatcher.c'),
        os.path.join(base, 'Core/Src/communication/puzzle.pb.c'),
        os.path.join(base, 'Core/third_party/nanopb/pb_common.c'),
        os.path.join(base, 'Core/third_party/nanopb/pb_decode.c'),
        os.path.join(base, 'Core/third_party/nanopb/pb_encode.c'),
        f'-I{base}/Core/Inc/communication',
        f'-I{base}/Core/third_party/nanopb',
        f'-I{base}/tests',
        '-lm',
    ]
    result = subprocess.run(compile_cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"FAIL: compile error:\n{result.stderr}")
        return 1

    # Create a PuzzleCommand using Python protobuf
    cmd = puzzle_pb2.PuzzleCommand()
    p0 = cmd.pieces.add()
    p0.piece_id = 5
    p0.pick_x = 123.45
    p0.pick_y = 678.9
    p0.place_x = 42.0
    p0.place_y = 99.9
    p0.rotation = 135.0

    p1 = cmd.pieces.add()
    p1.piece_id = 3
    p1.pick_x = 0.0
    p1.pick_y = 0.0
    p1.place_x = 210.0
    p1.place_y = 297.0
    p1.rotation = 0.0

    payload = cmd.SerializeToString()
    frame = struct.pack('>H', len(payload)) + payload

    # Pipe to C helper
    proc = subprocess.run(
        [helper_bin],
        input=frame,
        capture_output=True,
    )

    if proc.returncode != 0:
        print(f"FAIL: C helper returned {proc.returncode}")
        print(proc.stderr.decode())
        return 1

    # Parse C helper output (it prints decoded values as text)
    output = proc.stdout.decode().strip()
    print(f"C decoded output:\n{output}\n")

    # Also check the Ack that C sent back (appended as binary after the text)
    # The helper writes ack to stderr as raw bytes
    ack_frame = proc.stderr
    if len(ack_frame) >= 2:
        ack_len = struct.unpack('>H', ack_frame[:2])[0]
        ack = puzzle_pb2.Ack()
        ack.ParseFromString(ack_frame[2:2 + ack_len])
        print(f"Ack received: status={ack.status} (0=OK), piece_id={ack.piece_id}")
        if ack.status != puzzle_pb2.STATUS_OK:
            print("FAIL: Ack status not OK")
            return 1
    else:
        print("FAIL: No ack received")
        return 1

    # Verify values in text output
    if "piece_id=5" in output and "pick_x=123.4" in output and "rotation=135.0" in output:
        print("\nPASS")
        return 0
    else:
        print("\nFAIL: unexpected decoded values")
        return 1


if __name__ == '__main__':
    sys.exit(main())
