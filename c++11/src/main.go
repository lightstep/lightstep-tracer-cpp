package main

import (
    "github.com/lightstep/lightstep-tracer-go"
    "math"
)

func main() {
    // Initialize the LightStep Tracer; see lightstep.Options for tuning, etc.
    tracer := lightstep.NewTracer(lightstep.Options{
        AccessToken: "f5660bc5d76ca080cf36ea1d7881807a",
    })
    span := tracer.StartSpan("abc")
    x := math.NaN()
    y := math.Inf(1)
    z := math.Inf(-1)
    span.SetTag("nan", x)
    span.SetTag("+inf", y)
    span.SetTag("-inf", z)
    span.Finish()
    lightstep.FlushLightStepTracer(tracer)
}
