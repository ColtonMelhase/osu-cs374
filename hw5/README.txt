To compile the all files, run the compileall script

    compileall

Then to run the testing/grading script, run the following command. Where
$enc_port and $dec_port represent the ports the listening sockets for the
encryption and decryption servers respectively will be listening on.

    p5testscript $enc_port $dec_port

