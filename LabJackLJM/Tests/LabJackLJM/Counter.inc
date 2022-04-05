var signal = false
var count = 0 {
    report ('count = $count')
}
var previous_count = 0


%define num_trials = 100
var num_trials_done = 0


protocol (nsamples = 2) {
    task {
        state 'Begin State System' {
            signal = true
            count = -10
            num_trials_done = 0

            report ('Starting device')
            start_io_device (labjack)

            start_timer (
                timer = timer
                duration = 20ms
                )

            goto (
                target = 'Signal Low'
                when = count == 0
                )
            goto (
                target = 'Exit State System'
                when = timer_expired(timer)
                )
        }

        state 'Signal Low' {
            previous_count = count
            signal = false

            start_timer (
                timer = timer
                duration = 20ms
                )

            goto (
                target = 'Exit State System'
                when = count != previous_count
                )
            goto (
                target = 'Signal High'
                timer = timer_expired(timer)
                )
        }

        state 'Signal High' {
            signal = true

            start_timer (
                timer = timer
                duration = 20ms
                )

            goto (
                target = 'Counter incremented'
                when = count == previous_count + 1
                )
            goto (
                target = 'Exit State System'
                when = timer_expired(timer)
                )
        }

        state 'Counter incremented' {
            num_trials_done += 1

            goto (
                target = 'Exit State System'
                when = num_trials_done == num_trials
                )
            goto ('Signal Low')
        }

        state 'Exit State System' {
            report ('Stopping device')
            stop_io_device (labjack)

            assert (num_trials_done == num_trials)

            yield ()
        }
    }
}
