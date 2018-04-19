// some functions
$.fn.displaySpeed = function() {
  if ($(this).find("option:selected").data("speed"))
    $("div.speed").css("display", "block");
  else
    $("div.speed").css("display", "none");
  
  return this;
}

$.fn.displayColors = function() {
  let i = 0;
  let colors = [];
  
  $(this).find("option:selected").data("colors").split(",").forEach(value => {
    value = value.trim();
    if (value)
      colors.push(value);
  });
  
  $("div.color").each((key, value) => {
    $(value).find(".slider.slick-slider").slick("unslick");

    if (i < colors.length) {
      $(value).css("display", "block");
      $(value).find("h1").html(colors[i]);
      $(value).find(".slider").slickify();
    }
    else {
      $(value).css("display", "none");
    }
    
    i += 1;
  });
  
  return this;
};

$.fn.slickify = function() {
  $(this).each((key, value) => {
    let i = 0;
    $(value).find(".bg").each((k, v) => {
      if ($(v).data("color") == lastResponse.color[$(value).data("id")])
        i = k;
    });

    $(value).slick({
      infinite: true,
      prevArrow: false,
      nextArrow: false,
      initialSlide: i
    }).on("afterChange", updateColor);
  });
  
  return this;
};

// initialization
let lastResponse;

const update = function() {
  $.getJSON("/get", response => {
    console.log(response);
    lastResponse = response;
    
    $("select.mode").val(response.mode).displaySpeed().displayColors();
    $("input.speed").val(response.speed).trigger("input");
  }).fail(e => {
    console.log(e);
  });
}();

// mode
$("select.mode").change(function() {
  $.getJSON("/set?mode=" + $(this).val(), response => {
    console.log(response);
    lastResponse.mode = $(this).val();
    $(this).displaySpeed().displayColors();
  }).fail(e => {
    alert(JSON.stringify(e));
  });
});

// speed
$("input.speed").on("input", function() {
  $(this).parent().find("input.text").val($(this).val());
}).change(function() {
  $.getJSON("/set?speed=" + $(this).val(), response => {
    console.log(response);
    lastResponse.speed = $(this).val();
  }).fail(e => {
    alert(JSON.stringify(e));
  });
});

// color
let updateColorJobs = [ false, false, false, false ];
const updateColor = function(slick, current) {
  const id = $(this).data("id");
  const bg = $(this).find(".slick-current").data("color");

  window.clearTimeout(updateColorJobs[id]);
  updateColorJobs[id] = window.setTimeout(() => {
    $.getJSON("/set?color" + id + "=" + bg, response => {
      console.log(response);
      lastResponse.color[id] = bg;
    }).fail(e => {
      alert(JSON.stringify(e));
    });
  }, 1000);
};
