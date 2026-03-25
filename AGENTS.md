# AGENTS.md

## Purpose

This repository is worked on by coding agents.  
Your job is to implement the requested C++ changes correctly, minimally, and deterministically.

Do not improvise architecture.  
Do not replace requested functionality with a simpler different feature.  
Do not silently downgrade requirements just because the implementation is harder.

If the user asks for a real fix, implement the real fix.

---

## Core Rules

### 1. Stay inside scope
Only do the task that was requested.

- Do not refactor unrelated code.
- Do not rename files, classes, methods, variables, or folders unless required.
- Do not introduce abstractions, helper layers, wrappers, or “cleanup” unless necessary for the requested change.
- Do not “improve” adjacent systems without being asked.

### 2. Smallest real fix
Make the smallest change that actually solves the requested problem.

Good:
- fix the broken shader loading path
- add the required bgfx shader compilation step
- correct resource initialization order
- fix the actual API usage

Bad:
- remove shader usage because it is inconvenient
- replace a requested graphics feature with a flat color
- skip rendering work and claim a fallback is “simpler”
- rewrite the feature into something easier but different

### 3. Do not degrade the feature
Never replace the requested feature with a weaker substitute unless the user explicitly asks for a fallback.

Examples of forbidden behavior:
- replacing shaders with clear color
- replacing 3D rendering with UI mock output
- replacing async logic with blocking logic because it is easier
- replacing actual file IO with hardcoded data
- replacing a real parser with a stub
- disabling failing tests instead of fixing the code

If the task is “make X work”, your job is to make **X** work, not to remove X.

### 4. User is the ground truth
If the user says something is broken, assume it is broken.
Do not argue with the bug report.
Investigate and fix it.

### 5. No fake completion
Do not claim something works unless the code actually supports that claim.

Never say:
- “this should work now” without checking
- “bgfx does not support this” unless verified in the code/docs already present in the repo
- “simpler solution” when it changes the requirement
- “done” if there are known compile errors, missing assets, or unfinished integration points

### 6. No placeholders
Do not leave:
- TODOs
- stubs
- pseudocode
- commented-out future implementations
- fake fallback code pretending to be the final solution

Deliver complete working code.

---

## Decision Policy

When implementation is difficult, follow this order:

1. Understand the existing code and build flow
2. Identify the real root cause
3. Fix the root cause
4. Preserve the original requested behavior
5. Keep changes minimal

Do **not** do this:

1. Decide the requested thing is “too hard”
2. Invent a reduced alternative
3. Ship the reduced alternative
4. Present it as a smart solution

That is failure.

---

## C++ Specific Rules

### Build and compile discipline
Before finalizing, mentally verify at minimum:

- includes are correct
- forward declarations are valid
- namespaces are correct
- symbol names match declarations
- signatures match headers and implementations
- const correctness is not broken
- ownership and lifetime are sane
- required source files are included in the build
- paths and asset references are correct
- platform-specific code paths are guarded correctly

Do not introduce code that obviously cannot compile.

### Respect the existing style
- Match the repository’s existing C++ style.
- Do not reformat unrelated files.
- Do not switch brace style, naming style, include order style, or error handling style unless editing that exact area requires it.

### Respect current architecture
- Use existing systems first.
- Reuse current resource loading, logging, math, rendering, and platform layers where appropriate.
- Do not add a new framework or subsystem for a small task.

### Avoid speculative abstractions
Do not create:
- generic utility classes “for future use”
- base classes not needed by the task
- new interfaces just to look clean
- template machinery unless the task truly requires it

---

## Graphics / Engine / Rendering Rules

These are critical.

### Do not replace real rendering work with fake substitutes
If the task involves shaders, pipelines, materials, render passes, meshes, textures, or GPU resources:

- implement the real rendering path required by the engine/framework
- wire assets correctly
- compile or load shaders correctly for the project’s pipeline
- keep the intended visual feature intact

Forbidden shortcut pattern:
- “this API does not directly support X, so I replaced it with a simpler visual approximation”

That is only allowed if the user explicitly asks for a temporary fallback.

### For bgfx specifically
If working with bgfx:

- respect bgfx’s actual shader pipeline
- use the project’s real shader compilation flow if one exists
- if shader binaries are required, add or fix the correct build/integration step
- do not replace shader-based rendering with clear color, UI fill, or unrelated hacks just to avoid proper integration

If shader compilation or asset packaging is missing, fix that pipeline.  
Do not remove the feature to avoid the pipeline work.

### Preserve intent
If the requested result is:
- atmosphere
- lighting
- post-process
- skybox
- terrain shading
- material effect

then implement that feature properly within the engine constraints.  
Do not collapse it into “good enough” unless the user explicitly approves a fallback.

---

## When Blocked

If you hit a real blocker, do this:

1. State the exact blocker briefly
2. Point to the exact file / API / build step involved
3. Propose the minimal path that preserves the requested feature
4. Continue as far as possible without changing the requirement

Example of acceptable behavior:
- “bgfx in this project expects precompiled shaders from the shader build step. I updated the build to generate the required binaries and wired the loader to use them.”

Example of unacceptable behavior:
- “bgfx does not support direct GLSL loading, so I removed the shader feature and used a flat color instead.”

---

## Allowed Fallbacks

Fallbacks are allowed only when one of these is true:

- the user explicitly asks for a temporary workaround
- the user explicitly says correctness can be traded for speed
- the user explicitly asks for a mock, prototype, or placeholder

Even then:
- label the fallback clearly
- do not present it as the full implementation
- do not silently replace the original requirement

---

## File Editing Rules

- Edit only files relevant to the task.
- Keep diffs tight.
- Do not create random scratch files.
- Do not leave backup files, temp files, or “fixed2/final_final” copies.
- Put new files in the correct project location only when required.

---

## Testing and Verification

Before final output, verify as much as possible from the repository context:

- compile-time consistency
- references and includes
- build integration
- resource paths
- config correctness
- no obvious missing symbols
- no obviously broken control flow

If there are tests relevant to the task, update or add the smallest necessary coverage.

Do not disable tests to make the task appear complete.

---

## Output Rules

When reporting work:

- be precise
- say what changed
- mention important constraints
- mention any remaining real limitation honestly

Do not pad the answer with speculative advice.
Do not propose unrelated redesigns.
Do not celebrate partial work as completion.

---

## Hard Prohibitions

Never do any of the following unless the user explicitly asks:

- replace requested functionality with a simpler different one
- remove broken code without restoring the feature
- silently add a mock instead of a real implementation
- silently downgrade graphics, rendering, parsing, networking, or storage behavior
- skip required build pipeline work by hardcoding output
- invent new architecture to avoid understanding old code
- claim library limitations without verifying from project reality
- turn a real implementation request into a prototype

---

## One-line Operating Principle

Implement the requested thing as requested, using the project’s real C++ and build pipeline, with the smallest real fix, and without creative shortcuts.


## Rendering Anti-Slacking Rule

If a task requires a shader, render pass, material, or GPU effect, you must implement the actual rendering path needed by the project.

You are not allowed to:
- replace it with clear color
- replace it with an ImGui fullscreen rectangle
- replace it with a mock visual
- remove the effect because the asset/build pipeline needs work

If the pipeline is missing, fix the pipeline.
If the asset format is wrong, fix the asset format.
If loading is wrong, fix loading.
But keep the feature.


## READ ADDITIONAL INFO

Project info: `INSTRUCTIONS.md`