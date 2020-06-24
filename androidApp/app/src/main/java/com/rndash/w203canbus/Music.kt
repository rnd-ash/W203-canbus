package com.rndash.w203canbus

import kotlin.properties.Delegates

class Music {
    var trackLength: Int by Delegates.observable(0) { _, old, new ->
        if (old != new) {
            ConnectService.ic.sendTrackDuration(new / 1000) // we want seconds, not milliseconds
        }
    }

    var trackPosition: Int by Delegates.observable(0) { _, old, new ->
        if (old != new) {
            ConnectService.ic.sendTrackSeek(new / 1000) // we want seconds, not milliseconds
        }
    }

    var trackName: String by Delegates.observable("") { _, old, new ->
        if (old != new) {
            ConnectService.ic.sendTrackName(new)
        }
    }

    var artistName: String by Delegates.observable("") { _, old, new ->
        // TODO
    }

    var playStatus: Boolean by Delegates.observable(false) { _, old, new ->
        if (old != new) {
            ConnectService.ic.sendPlayStatus(new)
        }
    }

}