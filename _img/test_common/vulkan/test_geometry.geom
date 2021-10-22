#version 450

layout (points) in;
layout (line_strip, max_vertices = 2) out;

void GSMain() {    
  gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0); 
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4( 0.1, 0.0, 0.0, 0.0);
  EmitVertex();
  
  EndPrimitive();
} 
