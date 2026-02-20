Import("env")
import os

# .envファイルを読み込んでbuild_flagsに追加する
env_file = os.path.join(env.subst("$PROJECT_DIR"), ".env")

if os.path.exists(env_file):
    with open(env_file, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            key, _, value = line.partition("=")
            key = key.strip()
            value = value.strip().strip('"').strip("'")
            env.Append(CPPDEFINES=[(key, env.StringifyMacro(value))])
else:
    print("WARNING: .env file not found at %s" % env_file)
    print("         Copy .env.example to .env and fill in your values.")

