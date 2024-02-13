%require loop

%ifdef triggered
    %define trigger_line = FIO6
%else
    %define trigger_line = ''
%end


python_file ('get_data.py')


%define field_size = 20
%define pointer_size = 1

var output_min = 0.0
var output_max = 4.75
%define output_mean = (output_max + output_min) / 2
%define output_range = output_max - output_min


var data_interval = 5ms

var output1 = []
var output2 = ''

var input1 = 0.0
var input2 = 0.0

%ifdef triggered
    var trigger = false
%end


labjack_ljm labjack (
    update_interval = 10ms
    analog_waveform_data_interval = data_interval
    analog_waveform_trigger_line = trigger_line
    ) {
    labjack_ljm_analog_waveform (
        line = DAC0
        data = output1
        loop = loop
        )
    labjack_ljm_analog_waveform (
        line = DAC1
        value = output2
        loop = loop
        )
    labjack_ljm_analog_input (
        line = AIN2
        value = input1
        )
    labjack_ljm_analog_input (
        line = AIN3
        value = input2
        )
    %ifdef triggered
        labjack_ljm_digital_output (
            line = FIO4
            value = trigger
            )
    %end
}


stimulus_display (redraw_on_every_refresh = true)


rectangle bg (
    color = 0,0,0
    x_size = field_size + 1.5 * pointer_size
    )

circle pointer (
    x_size = pointer_size
    x_position = field_size * (input1 - output_mean) / output_range
    y_position = field_size * (input2 - output_mean) / output_range
    )


protocol {
    output1 = py_eval('get_data(3)')
    assert (type(output1) == 'list')

    output2 = py_eval('get_data(9).tobytes()')
    assert (type(output2) == 'string')

    input1 = output_min
    input2 = output_min

    %ifdef triggered
        trigger = false
    %end

    queue_stimulus (bg)
    live_queue_stimulus (pointer)
    update_display ()
    wait (2s)

    report ('Starting device')
    start_device_io (labjack)
    wait (2s)

    block (nsamples = 2) {
        report ('Starting analog waveform output')
        labjack_ljm_start_analog_waveform_output (labjack)
        wait (2s)

        %ifundef triggered
            wait (13s)
        %else
            report ('Setting trigger')
            trigger = true
            wait (3s)

            report ('Clearing trigger (should have no effect)')
            trigger = false
            wait (2s)

            report ('Setting trigger again (should have no effect)')
            trigger = true
            wait (2s)

            report ('Clearing trigger again (should have no effect)')
            trigger = false
            wait (8s)
        %end

        report ('Stopping analog waveform output')
        labjack_ljm_stop_analog_waveform_output (labjack)
        wait (2s)
    }

    report ('Stopping device')
    stop_device_io (labjack)
    wait (2s)
}
