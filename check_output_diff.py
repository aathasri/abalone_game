def main(num: int):
    """
    Check the difference between the output of the test and the reference output
    :param num: The number of the test
    """
    with open(f"Test{num}.board") as test_file:
        test_lines = set(test_file.readlines())

    with open(f"Test{num}_ref.board") as ref_file:
        ref_lines = set(ref_file.readlines())

    for line in ref_lines:
        if line not in test_lines:
            print(f"{line.strip()} WRONG in ref{num}")

    for line in test_lines:
        if line not in ref_lines:
            print(f"{line.strip()} WRONG in test{num}")

    print(f"Tests finished for Test{num}")


if __name__ == '__main__':
    # Run the tests for Test1.board and Test2.board
    main(1)
    main(2)
