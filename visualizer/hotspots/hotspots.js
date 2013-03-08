
var data = null;
var stats = null;
var show_cold_edges = false;


function redraw()
{
  if (data == null)
  {
    // not ready yet
    return;
  }

  draw();
}

function prepareStats()
{
  stats = {};

  stats.count_max = 0;

  for (var i=0; i<data.length; i++)
  {
    if (data[i].count > stats.count_max)
    {
      stats.count_max = data[i].count;
    }
  }
}

function getStyle(obj)
{
  if (stats == null)
  {
    throw "forgot to call prepareStats()"
  }

  // calculate hot threshold
  // arguably, this logic needs to be inside
  // the C++ code so that later optimizations can
  // know what to work on. But for now, it's
  // easier to dump it here

  var percentile = obj.count / stats.count_max;
  var red = Math.round(percentile*255);
  red = red.toString(16);
  if (red.length == 1)
  {
    // pad to len 2
    red = '0' + red;
  }

  var color = '#' + red + '0000';

  var thickness = percentile*4;

  return {
    'stroke': color,
    'fill': color+"|"+thickness,
    'label': obj.count.toString()
  };
}

function populateGraph(g)
{
  console.log('creating nodes and edges');

  prepareStats();

  for (var i=0; i<data.length; i++)
  {
    var count = data[i].count;
    if (!show_cold_edges && count < 0.05 * stats.count_max)
    {
      continue;
    }

    var from = data[i].calling;
    var to = data[i].callee;
    var style = getStyle(data[i]);

    g.addEdge(from, to, style);

    console.log("adding edge " + from + " -> " + to + 
        ' with style ' + JSON.stringify(style));
  }
}

function draw()
{
  console.log('drawing');

  $('#canvas').replaceWith($('<div id="canvas" />'));

  var g = new Graph();
  g.edgeFactory.template.style.directed = true;

  populateGraph(g);

  var layouter = new Graph.Layout.Ordered(g, topological_sort(g));

  var renderer = new Graph.Renderer.Raphael(
      'canvas', g, $(document).width(), $(document).height() - 100);

  layouter.layout();
  renderer.draw();
};


$(document).ready(function()
{
  $('#file').change(function(evt)
  {
    console.log('file ready, will load & parse');

    var fr = new FileReader();
    
    fr.onload = function(frEvt)
    {
      var txt = frEvt.target.result;
      console.log('read ' + txt.length + ' bytes');

      data = $.csv.toObjects(txt);
      console.log('csv parsed');
      console.log(data);

      // parse data types
      for (var i=0; i<data.length; i++)
      {
        data[i].count = parseInt(data[i].count);
      }

      redraw();
    };

    fr.readAsText(evt.target.files[0]);
  });



  $('#cb_cold_edges').change(function(evt)
  {
    show_cold_edges = evt.target.checked;
    redraw();
  });
});

