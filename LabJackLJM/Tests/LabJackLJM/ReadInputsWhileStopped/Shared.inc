var counter_signal = true
var quadrature_signal = 1

%define NO_VALUE = -100

var analog_input = NO_VALUE
var digital_input = NO_VALUE
var counter = NO_VALUE
var quadrature_input = NO_VALUE


%define labjack_channels ()
    labjack_ljm_analog_output (
        line = DAC0
        value = 4.75
        )
    labjack_ljm_digital_output (
        line = cio0
        value = true
        )
    labjack_ljm_digital_output (
        line = eio4
        value = counter_signal
        )
    labjack_ljm_word_output (
        first_line = fio4
        num_lines = 2
        value = quadrature_signal
        )

    labjack_ljm_analog_input (
        line = AIN2
        value = analog_input
        )
    labjack_ljm_digital_input (
        line = eio2
        value = digital_input
        )
    labjack_ljm_counter (
        line = cio2
        value = counter
        )
    labjack_ljm_quadrature_input (
        first_line = fio6
        value = quadrature_input
        )
%end


%define assert_analog_input (expected_value)
    assert (abs(analog_input - expected_value) < 0.005)
%end

%define assert_digital_input (expected_value)
    assert (digital_input == expected_value)
%end

%define assert_counter (expected_value)
    assert (counter == expected_value)
%end

%define assert_quadrature_input (expected_value)
    assert (quadrature_input == expected_value)
%end
